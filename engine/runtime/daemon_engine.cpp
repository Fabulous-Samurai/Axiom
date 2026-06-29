// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file daemon_engine.cpp
 * @brief Implementation of the DaemonEngine service (Operation PLUTO EXODUS)
 */

#include "daemon_engine.h"
#include "dynamic_calc.h"
#include "sentry.h"
#include "cpu_optimization.h"

#include <format>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <expected>
#include <string_view>
#include <charconv>
#include <ranges>

#ifndef _WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace AXIOM {

namespace {

// Utility to safely copy a string_view to a std::array<char, N>
template<size_t N>
void copy_sv_to_array(std::array<char, N>& dest, std::string_view src) noexcept {
    dest.fill('\0');
    std::memcpy(dest.data(), src.data(), std::min(src.size(), N - 1));
}

[[nodiscard]] uint32_t read_env_u32(const char* name, uint32_t fallback) noexcept
{
    const char* raw = std::getenv(name);
    if (raw == nullptr || *raw == '\0') return fallback;
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(raw, &end, 10);
    if (end == raw || *end != '\0' || parsed == 0UL) return fallback;
    return static_cast<uint32_t>(parsed);
}

[[nodiscard]] int64_t read_env_i64(const char* name, int64_t fallback) noexcept
{
    const char* raw = std::getenv(name);
    if (raw == nullptr || *raw == '\0') return fallback;
    char* end = nullptr;
    const long long parsed = std::strtoll(raw, &end, 10);
    if (end == raw || *end != '\0' || parsed <= 0LL) return fallback;
    return static_cast<int64_t>(parsed);
}

[[nodiscard]] uint32_t circuit_failure_threshold() noexcept
{
    static const uint32_t value = read_env_u32("AXIOM_DAEMON_CIRCUIT_FAILURE_THRESHOLD", 5U);
    return value;
}

[[nodiscard]] int64_t circuit_open_duration_ms() noexcept
{
    static const int64_t value = read_env_i64("AXIOM_DAEMON_CIRCUIT_OPEN_MS", 2000LL);
    return value;
}

[[nodiscard]] int64_t backpressure_wait_ms() noexcept
{
    static const int64_t value = read_env_i64("AXIOM_DAEMON_BACKPRESSURE_WAIT_MS", 5LL);
    return value;
}

[[nodiscard]] int64_t now_ms() noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

AXIOM_FORCE_INLINE void apply_mode_from_request(DynamicCalc& calc, std::string_view mode) noexcept
{
    if (mode == "linear" || mode == "linear_system") calc.SetMode(CalculationMode::LINEAR_SYSTEM);
    else if (mode == "stats" || mode == "statistics") calc.SetMode(CalculationMode::STATISTICS);
    else if (mode == "symbolic") calc.SetMode(CalculationMode::SYMBOLIC);
    else calc.SetMode(CalculationMode::ALGEBRAIC);
}

template<typename Queue>
AXIOM_FORCE_INLINE bool enqueue_until_deadline(Queue& queue, const DaemonEngine::Request& request, const std::chrono::steady_clock::time_point deadline, const std::atomic<bool>& running) noexcept
{
    while (std::chrono::steady_clock::now() < deadline) {
        if (queue.push(request)) return true;
        if (!running.load(std::memory_order::relaxed)) [[unlikely]] return false;
        AXIOM_YIELD_PROCESSOR();
    }
    return false;
}

} // namespace

DaemonEngine::DaemonEngine(std::string_view pipe_name) noexcept {
    copy_sv_to_array(pipe_name_, pipe_name);
}

DaemonEngine::~DaemonEngine() noexcept {
    if (running_.load(std::memory_order::acquire)) {
        stop();
    }
}

std::expected<void, DaemonEngine::DaemonStatus> DaemonEngine::start() noexcept {
    bool expected_run = false;
    if (!running_.compare_exchange_strong(expected_run, true, std::memory_order::acq_rel)) {
        return std::unexpected(status_.load(std::memory_order::acquire));
    }

    status_.store(DaemonStatus::STARTING, std::memory_order::release);

    auto result = setup_pipe();
    if (!result) {
        // Logging is now handled by the caller or a dedicated logger.
        running_.store(false, std::memory_order::release);
        status_.store(DaemonStatus::PIPE_ERROR, std::memory_order::release);
        return std::unexpected(DaemonStatus::PIPE_ERROR);
    }

    status_.store(DaemonStatus::READY, std::memory_order::release);
    daemon_thread_ = std::jthread([this] { daemon_loop(); });
    request_processor_ = std::jthread([this] { request_processor_loop(); });
    return {};
}

void DaemonEngine::stop() noexcept {
    bool expected_run = true;
    if (!running_.compare_exchange_strong(expected_run, false, std::memory_order::acq_rel)) {
        return;
    }

    status_.store(DaemonStatus::SHUTDOWN, std::memory_order::release);

    // The jthread destructor will automatically join.
}

std::expected<void, DaemonEngine::PipeError> DaemonEngine::setup_pipe() noexcept {
#ifdef _WIN32
    wchar_t wide_pipe_name[256 + 12];
    swprintf(wide_pipe_name, sizeof(wide_pipe_name)/sizeof(wchar_t), L"\.\pipe\%hs", pipe_name_.data());

    pipe_handle_ = CreateNamedPipeW(
        wide_pipe_name,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 4096, 4096, 0, nullptr);

    if (pipe_handle_ == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) return std::unexpected(PipeError::PermissionDenied);
        if (err == ERROR_ALREADY_EXISTS) return std::unexpected(PipeError::AlreadyExists);
        return std::unexpected(PipeError::SystemError);
    }
    return {};
#else
    char path[256 + 6];
    snprintf(path, sizeof(path), "/tmp/%s", pipe_name_.data());
    ::unlink(path);
    if (::mkfifo(path, 0600) != 0) {
        if (errno == EACCES) return std::unexpected(PipeError::PermissionDenied);
        return std::unexpected(PipeError::SystemError);
    }
    pipe_fd_ = ::open(path, O_RDONLY | O_NONBLOCK);
    if (pipe_fd_ < 0) return std::unexpected(PipeError::SystemError);
    return {};
#endif
}

void DaemonEngine::cleanup_pipe() noexcept {
#ifdef _WIN32
    if (pipe_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (pipe_fd_ >= 0) {
        ::close(pipe_fd_);
        pipe_fd_ = -1;
    }
    char path[256 + 6];
    snprintf(path, sizeof(path), "/tmp/%s", pipe_name_.data());
    ::unlink(path);
#endif
}

const char* DaemonEngine::pipe_error_to_string(PipeError error) noexcept {
    switch (error) {
        case PipeError::None: return "None";
        case PipeError::PermissionDenied: return "PermissionDenied";
        case PipeError::AlreadyExists: return "AlreadyExists";
        // ... other cases
        default: return "UnknownError";
    }
}

static std::string_view json_get_view(std::string_view json, std::string_view key) noexcept {
    // This is a placeholder for actual JSON parsing logic.
    // In a real scenario, this would parse a JSON string to extract the value associated with the key.
    // For the purpose of this daemon, we'll return empty string_view.
    return {};
}

AXIOM_FORCE_INLINE bool decode_request(std::string_view req_str, std::atomic<uint64_t>& next_request_id, DaemonEngine::Request& out) noexcept {
    auto cmd = json_get_view(req_str, "command");
    auto sid = json_get_view(req_str, "session");
    auto mod = json_get_view(req_str, "mode");

    if (cmd.empty()) return false;

    copy_sv_to_array(out.command, cmd);
    copy_sv_to_array(out.session_id, sid);
    if (!mod.empty()) copy_sv_to_array(out.mode, mod);

    out.request_id = next_request_id.fetch_add(1, std::memory_order::relaxed);
    out.timestamp = std::chrono::steady_clock::now();
    return true;
}

void DaemonEngine::daemon_loop() noexcept {
    // Placeholder implementation for the main daemon loop
    // In a real scenario, this would handle pipe connections and dispatch requests.
    while (running_.load(std::memory_order::acquire)) {
        // Check for incoming requests
        // If request, decode and push to request_queue_
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        AXIOM_YIELD_PROCESSOR();
    }
}

void DaemonEngine::request_processor_loop() noexcept {
    // Placeholder implementation for processing requests from the queue
    while (running_.load(std::memory_order::acquire)) {
        Request req;
        if (request_queue_.pop(req)) {
            Response resp = execute_command(req);
            // Send response back via pipe
            // For now, just discard response
            update_metrics(resp.execution_time_ms); // Update metrics even if response is discarded
            completed_requests_.fetch_add(1, std::memory_order::relaxed);
        }
        AXIOM_YIELD_PROCESSOR();
    }
}

DaemonEngine::Response DaemonEngine::execute_command(const Request& req) noexcept {
    Response resp{};
    resp.request_id = req.request_id;
    copy_sv_to_array(resp.session_id, std::string_view(req.session_id.data()));
    resp.timestamp = std::chrono::steady_clock::now();

    thread_local DynamicCalc calc;
    apply_mode_from_request(calc, std::string_view(req.mode.data()));
    auto engine_result = calc.Evaluate(std::string_view(req.command.data()));

    if (engine_result.HasErrors()) {
        resp.success = false;
        copy_sv_to_array(resp.error, "EngineError");
    } else {
        resp.success = true;
        auto d = engine_result.GetDouble();
        if (d) {
            char buf[64];
            auto [ptr, ec] = std::to_chars(buf, buf + 64, *d);
            if (ec == std::errc()) copy_sv_to_array(resp.result, std::string_view(buf, ptr - buf));
            else copy_sv_to_array(resp.result, "error:to_chars");
        } else {
            copy_sv_to_array(resp.result, "ok");
        }
    }
    return resp;
}

void DaemonEngine::update_metrics(double execution_time) noexcept {
    // Placeholder for metric updates
    (void)execution_time;
}

DaemonEngine::Response DaemonEngine::process_request(const Request& request) noexcept {
    return execute_command(request);
}

bool DaemonEngine::send_command(std::string_view session_id, std::string_view command, std::string_view mode) noexcept {
    if (!running_.load(std::memory_order::relaxed)) return false;

    Request req{};
    copy_sv_to_array(req.session_id, session_id);
    copy_sv_to_array(req.command, command);
    copy_sv_to_array(req.mode, mode);
    req.request_id = next_request_id_.fetch_add(1, std::memory_order::relaxed);
    req.timestamp = std::chrono::steady_clock::now();

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(backpressure_wait_ms());
    if (!enqueue_until_deadline(request_queue_, req, deadline, running_)) {
        rejected_requests_.fetch_add(1, std::memory_order::relaxed);
        return false;
    }
    enqueued_requests_.fetch_add(1, std::memory_order::relaxed);
    return true;
}

std::expected<std::string_view, std::error_code> DaemonEngine::create_session() noexcept {
    std::scoped_lock lock(sessions_mutex_);
    if (sessions_.full()) {
        return std::unexpected(std::make_error_code(std::errc::too_many_files_open));
    }

    auto ctx = std::make_unique<SessionContext>();
    auto ns = std::chrono::steady_clock::now().time_since_epoch().count();
    auto res = std::format_to(ctx->session_id.data(), "session_{}", ns);
    *res = '\0';

    ctx->created_at = std::chrono::steady_clock::now();

    std::string_view id_view(ctx->session_id.data());
    sessions_.emplace_back(id_view, std::move(ctx));

    return id_view;
}

bool DaemonEngine::destroy_session(std::string_view session_id) noexcept {
    std::scoped_lock lock(sessions_mutex_);
    auto it = std::ranges::find_if(sessions_, [&](const auto& pair) {
        return pair.first == session_id;
    });

    if (it != sessions_.end()) {
        sessions_.erase(it);
        return true;
    }
    return false;
}

void DaemonEngine::get_active_sessions(FixedVector<std::string_view, 128>& out_sessions) noexcept {
    std::scoped_lock lock(sessions_mutex_);
    out_sessions.clear();
    for (const auto& [id, _] : sessions_) {
        out_sessions.push_back(id);
    }
}

std::chrono::milliseconds DaemonEngine::get_uptime() const noexcept {
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startup_time_);
}

} // namespace AXIOM
