// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file daemon_engine.cpp
 * @brief AXIOM Engine v3.1 - Hardened Daemon Implementation
 *
 * Core updates:
 * - IPC: SPSC Lock-free ring buffer integration.
 * - Concurrency: OS-bypass spin-wait mechanisms via std::this_thread::yield().
 * - Memory: Rvalue references deployed for zero-copy request dispatch.
 */

#include "../include/daemon_engine.h"
#include "../include/dynamic_calc.h"
#include "../include/sentry.h"
#include "cpu_optimization.h"

#include <iostream>
#include <format>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <type_traits>
#include <variant>
#include <expected>

#ifndef _WIN32
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace AXIOM {

namespace {

[[nodiscard]] uint32_t read_env_u32(const char* name, uint32_t fallback) noexcept
{
    const char* raw = std::getenv(name);
    if (raw == nullptr || *raw == '\0')
    {
        return fallback;
    }

    char* end = nullptr;
    const unsigned long parsed = std::strtoul(raw, &end, 10);
    if (end == raw || *end != '\0' || parsed == 0UL)
    {
        return fallback;
    }

    return static_cast<uint32_t>(parsed);
}

[[nodiscard]] int64_t read_env_i64(const char* name, int64_t fallback) noexcept
{
    const char* raw = std::getenv(name);
    if (raw == nullptr || *raw == '\0')
    {
        return fallback;
    }

    char* end = nullptr;
    const long long parsed = std::strtoll(raw, &end, 10);
    if (end == raw || *end != '\0' || parsed <= 0LL)
    {
        return fallback;
    }

    return static_cast<int64_t>(parsed);
}

[[nodiscard]] uint32_t circuit_failure_threshold() noexcept
{
    static const uint32_t value =
        read_env_u32("AXIOM_DAEMON_CIRCUIT_FAILURE_THRESHOLD", 5U);
    return value;
}

[[nodiscard]] int64_t circuit_open_duration_ms() noexcept
{
    static const int64_t value =
        read_env_i64("AXIOM_DAEMON_CIRCUIT_OPEN_MS", 2000LL);
    return value;
}

[[nodiscard]] int64_t backpressure_wait_ms() noexcept
{
    static const int64_t value =
        read_env_i64("AXIOM_DAEMON_BACKPRESSURE_WAIT_MS", 5LL);
    return value;
}

[[nodiscard]] int64_t now_ms() noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

AXIOM_FORCE_INLINE void apply_mode_from_request(DynamicCalc& calc, const std::string& mode)
{
    if (mode == "linear" || mode == "linear_system")
    {
        calc.SetMode(CalculationMode::LINEAR_SYSTEM);
        return;
    }
    if (mode == "stats" || mode == "statistics")
    {
        calc.SetMode(CalculationMode::STATISTICS);
        return;
    }
    if (mode == "symbolic")
    {
        calc.SetMode(CalculationMode::SYMBOLIC);
        return;
    }
    calc.SetMode(CalculationMode::ALGEBRAIC);
}

template<typename Queue>
AXIOM_FORCE_INLINE bool enqueue_until_deadline(Queue& queue,
                                               const DaemonEngine::Request& request,
                                               const std::chrono::steady_clock::time_point deadline,
                                               const std::atomic<bool>& running) noexcept
{
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (queue.push(request))
        {
            return true;
        }
        if (!running.load(std::memory_order_acquire)) [[unlikely]]
        {
            return false;
        }
        AXIOM_YIELD_PROCESSOR;
    }
    return false;
}

[[nodiscard]] const char* calc_error_to_string(CalcErr err) noexcept
{
    switch (err)
    {
        case CalcErr::None: return "None";
        case CalcErr::DivideByZero: return "DivideByZero";
        case CalcErr::IndeterminateResult: return "IndeterminateResult";
        case CalcErr::OperationNotFound: return "OperationNotFound";
        case CalcErr::ArgumentMismatch: return "ArgumentMismatch";
        case CalcErr::NegativeRoot: return "NegativeRoot";
        case CalcErr::DomainError: return "DomainError";
        case CalcErr::ParseError: return "ParseError";
        case CalcErr::NumericOverflow: return "NumericOverflow";
        case CalcErr::StackOverflow: return "StackOverflow";
        case CalcErr::MemoryExhausted: return "MemoryExhausted";
        case CalcErr::InfiniteLoop: return "InfiniteLoop";
        default: return "UnknownCalcErr";
    }
}

[[nodiscard]] const char* linalg_error_to_string(LinAlgErr err) noexcept
{
    switch (err)
    {
        case LinAlgErr::None: return "None";
        case LinAlgErr::NoSolution: return "NoSolution";
        case LinAlgErr::InfiniteSolutions: return "InfiniteSolutions";
        case LinAlgErr::MatrixMismatch: return "MatrixMismatch";
        case LinAlgErr::ParseError: return "ParseError";
        default: return "UnknownLinAlgErr";
    }
}

[[nodiscard]] const char* engine_error_to_string(const EngineErrorResult& err) noexcept
{
    return std::visit([](const auto& e) -> const char* {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, CalcErr>)
            return calc_error_to_string(e);
        if constexpr (std::is_same_v<T, LinAlgErr>)
            return linalg_error_to_string(e);
        return "UnknownEngineError";
    }, err);
}

} // namespace

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

DaemonEngine::DaemonEngine(const std::string& pipe_name)
    : pipe_name_(pipe_name)
{
}

DaemonEngine::~DaemonEngine() noexcept
{
    if (!running_.load(std::memory_order_acquire))
    {
        return;
    }

    stop();
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

std::expected<void, DaemonEngine::DaemonStatus> DaemonEngine::start()
{
    bool expected_run = false;
    if (!running_.compare_exchange_strong(expected_run, true,
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
    {
        return std::unexpected(status_.load(std::memory_order_acquire)); // already running
    }

    status_.store(DaemonStatus::STARTING, std::memory_order_release);

    auto result = setup_pipe();
    if (!result)
    {
        std::cerr << "[AXIOM Daemon] setup_pipe failed: "
                  << pipe_error_to_string(result.error()) << '\n';
        running_.store(false, std::memory_order_release);
        status_.store(DaemonStatus::PIPE_ERROR, std::memory_order_release);
        return std::unexpected(DaemonStatus::PIPE_ERROR);
    }

    status_.store(DaemonStatus::READY, std::memory_order_release);
    daemon_thread_    = std::jthread([this] { 
        CPUOptimization::SetThreadAffinity(2); // Pin to core 2
        daemon_loop(); 
    });
    request_processor_ = std::jthread([this] { 
        CPUOptimization::SetThreadAffinity(3); // Pin to core 3
        request_processor_loop(); 
    });
    return {};
}

void DaemonEngine::stop() noexcept
{
    bool expected_run = true;
    if (!running_.compare_exchange_strong(expected_run, false,
                                          std::memory_order_release,
                                          std::memory_order_relaxed))
    {
        return;
    }

    status_.store(DaemonStatus::SHUTDOWN, std::memory_order_release);

    if (daemon_thread_.joinable())
    {
        daemon_thread_.join();
    }
    if (request_processor_.joinable())
    {
        request_processor_.join();
    }

    cleanup_pipe();

    std::scoped_lock lock(sessions_mutex_);
    sessions_.clear();
}

// ---------------------------------------------------------------------------
// Pipe setup / teardown
// ---------------------------------------------------------------------------

std::expected<void, DaemonEngine::PipeError> DaemonEngine::setup_pipe()
{
#ifdef _WIN32
    std::wstring full_name = L"\\\\.\\pipe\\" + std::wstring(pipe_name_.begin(), pipe_name_.end());
    pipe_handle_ = CreateNamedPipeW(
        full_name.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,       // max instances
        4096,    // out buffer
        4096,    // in buffer
        0,       // default timeout
        nullptr  // default security
    );
    if (pipe_handle_ == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_ACCESS_DENIED)    return std::unexpected(PipeError::PermissionDenied);
        if (err == ERROR_ALREADY_EXISTS)   return std::unexpected(PipeError::AlreadyExists);
        return std::unexpected(PipeError::SystemError);
    }
    return {};
#else
    std::string path = std::format("/tmp/{}", pipe_name_);
    // Remove stale FIFO
    ::unlink(path.c_str());
    if (::mkfifo(path.c_str(), 0600) != 0)
    {
        if (errno == EACCES) return PipeError::PermissionDenied;
        return PipeError::SystemError;
    }
    pipe_fd_ = ::open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (pipe_fd_ < 0) return PipeError::SystemError;
    return PipeError::None;
#endif
}

void DaemonEngine::cleanup_pipe()
{
#ifdef _WIN32
    if (pipe_handle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pipe_handle_);
        pipe_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (pipe_fd_ >= 0)
    {
        ::close(pipe_fd_);
        pipe_fd_ = -1;
    }
    std::string path = std::format("/tmp/{}", pipe_name_);
    ::unlink(path.c_str());
#endif
}

const char* DaemonEngine::pipe_error_to_string(PipeError error) noexcept
{
    switch (error) {
        case PipeError::None:                    return "None";
        case PipeError::PermissionDenied:        return "PermissionDenied";
        case PipeError::AlreadyExists:           return "AlreadyExists";
        case PipeError::ResourceExhausted:       return "ResourceExhausted";
        case PipeError::InvalidName:             return "InvalidName";
        case PipeError::SystemError:             return "SystemError";
        case PipeError::SecurityDescriptorFailed:return "SecurityDescriptorFailed";
        default:                                 return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Lightweight JSON key extractor — finds "key":"value" patterns.
// Expected format: {"command":"2+2","session":"abc","mode":"algebraic"}
#include <charconv>
#include <string_view>

// Zero-allocation JSON key extractor using string_view
static std::string_view json_get_view(std::string_view json, std::string_view key) noexcept
{
    char search_buf[64];
    // Simple manual join to avoid allocation
    size_t klen = key.size();
    if (klen > 50) return {};
    
    std::memcpy(search_buf, "\"", 1);
    std::memcpy(search_buf + 1, key.data(), klen);
    std::memcpy(search_buf + 1 + klen, "\":\"", 3);
    
    std::string_view search(search_buf, klen + 4);
    auto pos = json.find(search);
    if (pos == std::string_view::npos) return {};
    pos += search.size();
    auto end = json.find('"', pos);
    if (end == std::string_view::npos) return {};
    return json.substr(pos, end - pos);
}

// Fixed-capacity string escape (Zero-allocation)
template<size_t N>
static void json_escape_to(std::string_view in, FixedVector<char, N>& out) noexcept
{
    for (const char ch : in)
    {
        if (out.size() + 2 >= N) break;
        switch (ch)
        {
            case '"': out.push_back('\\'); out.push_back('"'); break;
            case '\\': out.push_back('\\'); out.push_back('\\'); break;
            case '\n': out.push_back('\\'); out.push_back('n'); break;
            case '\r': out.push_back('\\'); out.push_back('r'); break;
            case '\t': out.push_back('\\'); out.push_back('t'); break;
            default: out.push_back(ch); break;
        }
    }
}

AXIOM_FORCE_INLINE bool decode_request(const std::string& req_str,
                                       std::atomic<uint64_t>& next_request_id,
                                       DaemonEngine::Request& out) noexcept
{
    std::string_view sv(req_str);
    auto cmd = json_get_view(sv, "command");
    auto sid = json_get_view(sv, "session");
    auto mod = json_get_view(sv, "mode");
    
    if (cmd.empty()) [[unlikely]] return false;
    
    out.command = std::string(cmd); // These stay string for now as Request struct owns them
    out.session_id = std::string(sid);
    out.mode = std::string(mod);
    
    out.request_id = next_request_id.fetch_add(1, std::memory_order_relaxed);
    out.timestamp = std::chrono::steady_clock::now();
    return true;
}

static void build_response_json_fixed(const DaemonEngine::Response& resp, FixedVector<char, 4096>& out) noexcept
{
    auto append = [&](std::string_view s) {
        for (char c : s) if (out.size() < 4096) out.push_back(c);
    };

    append("{\"success\":");
    append(resp.success ? "true" : "false");
    append(",\"result\":\"");
    
    FixedVector<char, 2048> escaped_res;
    json_escape_to(resp.result, escaped_res);
    for (size_t i = 0; i < escaped_res.size(); ++i) out.push_back(escaped_res[i]);

    append("\",\"error\":\"");
    FixedVector<char, 512> escaped_err;
    json_escape_to(resp.error, escaped_err);
    for (size_t i = 0; i < escaped_err.size(); ++i) out.push_back(escaped_err[i]);

    append("\",\"time\":");
    char time_buf[32];
    auto [ptr, ec] = std::to_chars(time_buf, time_buf + 32, resp.execution_time_ms);
    if (ec == std::errc()) append(std::string_view(time_buf, ptr - time_buf));
    else append("0.0");

    append("}");
}

// ---------------------------------------------------------------------------
// IPC loops
// ---------------------------------------------------------------------------

void DaemonEngine::daemon_loop()
{
    while (running_.load(std::memory_order_acquire))
    {
        AXIOM::Sentry::instance().heartbeat_core();
#ifdef _WIN32
        process_windows_pipe();
#else
        process_posix_pipe();
#endif
    }
}

void DaemonEngine::process_windows_pipe()
{
#ifdef _WIN32
    if (pipe_handle_ == INVALID_HANDLE_VALUE) return;

    OVERLAPPED overlapped{};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!overlapped.hEvent) return;

    // Start non-blocking connection wait
    BOOL connected = ConnectNamedPipe(pipe_handle_, &overlapped);
    
    if (!connected) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            // Wait for client with 100ms timeout to avoid blocking main loop
            if (WaitForSingleObject(overlapped.hEvent, 100) != WAIT_OBJECT_0) {
                CancelIo(pipe_handle_);
                CloseHandle(overlapped.hEvent);
                return;
            }
        } else if (err != ERROR_PIPE_CONNECTED) {
            CloseHandle(overlapped.hEvent);
            return;
        }
    }

    char buffer[4096];
    DWORD bytes_read = 0;

    if (!ReadFile(pipe_handle_, buffer, sizeof(buffer) - 1, &bytes_read, nullptr))
    {
        DisconnectNamedPipe(pipe_handle_);
        CloseHandle(overlapped.hEvent);
        return; // Wait for next connection
    }

    buffer[bytes_read] = '\0';
    const std::string req_str(buffer);

    Request request;
    if (decode_request(req_str, next_request_id_, request))
    {
        const auto resp = execute_command(request);
        
        FixedVector<char, 4096> resp_json;
        build_response_json_fixed(resp, resp_json);

        DWORD written = 0;
        WriteFile(pipe_handle_,
                  resp_json.data(),
                  static_cast<DWORD>(resp_json.size()),
                  &written,
                  nullptr);
    }
    DisconnectNamedPipe(pipe_handle_);
    CloseHandle(overlapped.hEvent);
#endif
}

void DaemonEngine::process_posix_pipe()
{
#ifndef _WIN32
    char buffer[4096];
    ssize_t bytes_read = ::read(pipe_fd_, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return;
    }

    buffer[bytes_read] = '\0';
    const std::string req_str(buffer);

    Request request;
    if (decode_request(req_str, next_request_id_, request))
    {
        static constexpr auto kLoopBudget = std::chrono::milliseconds(2);
        const auto deadline = std::chrono::steady_clock::now() + kLoopBudget;
        if (!enqueue_until_deadline(request_queue_, request, deadline, running_))
        {
            rejected_requests_.fetch_add(1, std::memory_order_relaxed);
        }
        else
        {
            enqueued_requests_.fetch_add(1, std::memory_order_relaxed);
        }
    }
#endif
}

void DaemonEngine::request_processor_loop()
{
    while (running_.load(std::memory_order_acquire))
    {
        Request request;

        while (!request_queue_.pop(request)) {
            if (!running_.load(std::memory_order_acquire)) return;
            std::this_thread::yield();
        }

        status_.store(DaemonStatus::BUSY, std::memory_order_release);
        auto resp = execute_command(request);
        (void)resp; // async path: responses for send_command() callers are not piped back
        completed_requests_.fetch_add(1, std::memory_order_relaxed);

        total_requests_.fetch_add(1, std::memory_order_relaxed);
        // Fairness contract mirror: each started request reaches terminal state and daemon goes idle/ready.
        status_.store(DaemonStatus::READY, std::memory_order_release);
    }
}

// ---------------------------------------------------------------------------
// Command execution
// ---------------------------------------------------------------------------

DaemonEngine::Response DaemonEngine::execute_command(const Request& req)
{
    Response resp;
    resp.request_id        = req.request_id;
    resp.session_id        = req.session_id;
    resp.timestamp         = std::chrono::steady_clock::now();

    auto t0 = std::chrono::high_resolution_clock::now();
    
    const auto open_until = circuit_open_until_ms_.load(std::memory_order_acquire);
    const auto now = now_ms();
    if (open_until > now)
    {
        resp.success = false;
        resp.error = "CircuitBreakerOpen";
        rejected_requests_.fetch_add(1, std::memory_order_relaxed);
        return resp;
    }

    thread_local DynamicCalc calc;

    apply_mode_from_request(calc, req.mode);

    auto engine_result = calc.Evaluate(req.command);

    if (engine_result.HasErrors())
    {
        resp.success = false;
        if (engine_result.error.has_value())
        {
            resp.error = engine_error_to_string(engine_result.error.value());
        }
        else
        {
            resp.error = "EngineError";
        }
    }
    else
    {
        resp.success = true;
        auto d = engine_result.GetDouble();
        if (d) {
            char buf[64];
            auto [ptr, ec] = std::to_chars(buf, buf + 64, *d);
            if (ec == std::errc()) resp.result = std::string(buf, ptr - buf);
            else resp.result = "error:to_chars";
        } else {
            resp.result = "ok";
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    resp.execution_time_ms =
        std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (resp.success)
    {
        consecutive_failures_.store(0, std::memory_order_release);
    }
    else
    {
        const auto failures = consecutive_failures_.fetch_add(1, std::memory_order_acq_rel) + 1;
        if (failures >= circuit_failure_threshold())
        {
            circuit_open_until_ms_.store(now_ms() + circuit_open_duration_ms(), std::memory_order_release);
            consecutive_failures_.store(0, std::memory_order_release);
        }
    }

    update_metrics(resp.execution_time_ms);
    return resp;
}

void DaemonEngine::update_metrics(double execution_time) noexcept
{
    const uint64_t n = total_requests_.load(std::memory_order_relaxed) + 1;
    const double current_avg = avg_response_time_.load(std::memory_order_relaxed);
    const double new_avg = current_avg + (execution_time - current_avg) / static_cast<double>(n);
    avg_response_time_.store(new_avg, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

DaemonEngine::Response DaemonEngine::process_request(const Request& request)
{
    return execute_command(request);
}

bool DaemonEngine::send_command(const std::string& session_id,
                                const std::string& command,
                                const std::string& mode)
{
    if (!running_.load(std::memory_order_acquire)) [[unlikely]] return false;

    const auto open_until = circuit_open_until_ms_.load(std::memory_order_acquire);
    if (open_until > now_ms()) [[unlikely]]
    {
        rejected_requests_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    Request req{};
    req.session_id = session_id;
    req.command    = command;
    req.mode       = mode;
    req.request_id = next_request_id_.fetch_add(1, std::memory_order_relaxed);
    req.timestamp  = std::chrono::steady_clock::now();

    // Apply bounded backpressure: fail fast if queue remains saturated.
    const auto deadline = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(backpressure_wait_ms());
    if (!enqueue_until_deadline(request_queue_, req, deadline, running_)) [[unlikely]] {
        rejected_requests_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    enqueued_requests_.fetch_add(1, std::memory_order_relaxed);
    return true;
}

std::expected<std::string, std::error_code> DaemonEngine::create_session()
{
    // Use a nanosecond timestamp for a unique-enough ID without external deps
    auto ns = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string id = std::format("session_{}", ns);

    auto ctx = std::make_unique<SessionContext>();
    ctx->session_id  = id;
    ctx->created_at  = std::chrono::steady_clock::now();

    std::scoped_lock lock(sessions_mutex_);
    sessions_[id] = std::move(ctx);
    return id;
}

bool DaemonEngine::destroy_session(const std::string& session_id)
{
    std::scoped_lock lock(sessions_mutex_);
    return sessions_.erase(session_id) > 0;
}

void DaemonEngine::get_active_sessions(FixedVector<std::string, 128>& out_sessions)
{
    std::scoped_lock lock(sessions_mutex_);
    out_sessions.clear();
    for (const auto& [k, _] : sessions_) {
        if (out_sessions.size() >= 128) break;
        out_sessions.push_back(k);
    }
}

std::chrono::milliseconds DaemonEngine::get_uptime() const noexcept
{
    const auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startup_time_);
}

} // namespace AXIOM

