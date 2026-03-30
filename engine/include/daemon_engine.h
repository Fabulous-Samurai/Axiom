// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file daemon_engine.h
 * @brief AXIOM Engine v3.1 - Enterprise Daemon Mode Architecture
 */

#pragma once

#include "fixed_vector.h"
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <array>
#include <expected>
#include <system_error>
#include <string_view>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

namespace AXIOM {

/**
 * @brief High-performance Single-Producer Single-Consumer Lock-Free Queue
 */
template<typename T, size_t Capacity>
class LockFreeRingBuffer {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    
    T buffer_[Capacity];
    alignas(64) std::atomic<size_t> head_{0}; 
    alignas(64) std::atomic<size_t> tail_{0}; 

public:
    bool push(T item) noexcept {
        const size_t current_tail = tail_.load(std::memory_order::seq_cst);
        const size_t current_head = head_.load(std::memory_order::seq_cst);
        
        if (current_head - current_tail >= Capacity) [[unlikely]] return false; 
        
        buffer_[current_head & (Capacity - 1)] = std::move(item);
        head_.store(current_head + 1, std::memory_order::seq_cst);
        
        return true;
    }

    bool pop(T& item) noexcept {
        const size_t current_head = head_.load(std::memory_order::seq_cst);
        const size_t current_tail = tail_.load(std::memory_order::seq_cst);
        
        if (current_head == current_tail) [[unlikely]] return false; 
        
        item = std::move(buffer_[current_tail & (Capacity - 1)]);
        tail_.store(current_tail + 1, std::memory_order::seq_cst);
        
        return true;
    }
};

class DaemonEngine {
public:
    struct Request {
        std::array<char, 64> session_id{};
        std::array<char, 1024> command{};
        std::array<char, 32> mode{"algebraic"};
        std::chrono::steady_clock::time_point timestamp{};
        uint64_t request_id{0};
    };

    struct Response {
        uint64_t request_id{0};
        bool success{false};
        std::array<char, 1024> result{};
        std::array<char, 256> error{};
        double execution_time_ms{0.0};
        std::array<char, 64> session_id{};
        std::chrono::steady_clock::time_point timestamp{};
    };

    enum class DaemonStatus { STARTING, READY, BUSY, PIPE_ERROR, SHUTDOWN };
    enum class PipeError { None, PermissionDenied, AlreadyExists, ResourceExhausted, InvalidName, SystemError, SecurityDescriptorFailed, UnknownError };

private:
    template<size_t Capacity>
    class RequestQueueSoA {
        static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

        alignas(64) std::atomic<size_t> head_{0};
        alignas(64) std::atomic<size_t> tail_{0};

        std::array<std::array<char, 64>, Capacity> session_ids_{};
        std::array<std::array<char, 1024>, Capacity> commands_{};
        std::array<std::array<char, 32>, Capacity> modes_{};
        std::array<std::chrono::steady_clock::time_point, Capacity> timestamps_{};
        std::array<uint64_t, Capacity> request_ids_{};

    public:
        [[nodiscard]] bool push(const Request& item) noexcept {
            const size_t current_tail = tail_.load(std::memory_order::seq_cst);
            const size_t current_head = head_.load(std::memory_order::seq_cst);

            if (current_head - current_tail >= Capacity) [[unlikely]] return false;

            const size_t idx = current_head & (Capacity - 1);
            session_ids_[idx] = item.session_id;
            commands_[idx] = item.command;
            modes_[idx] = item.mode;
            timestamps_[idx] = item.timestamp;
            request_ids_[idx] = item.request_id;
            head_.store(current_head + 1, std::memory_order::seq_cst);
            return true;
        }

        [[nodiscard]] bool pop(Request& item) noexcept {
            const size_t current_head = head_.load(std::memory_order::seq_cst);
            const size_t current_tail = tail_.load(std::memory_order::seq_cst);

            if (current_head == current_tail) [[unlikely]] return false;

            const size_t idx = current_tail & (Capacity - 1);
            item.session_id = session_ids_[idx];
            item.command = commands_[idx];
            item.mode = modes_[idx];
            item.timestamp = timestamps_[idx];
            item.request_id = request_ids_[idx];
            tail_.store(current_tail + 1, std::memory_order::seq_cst);
            return true;
        }
    };

    std::atomic<DaemonStatus> status_{DaemonStatus::STARTING};
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> next_request_id_{1};
    
    std::array<char, 256> pipe_name_{};
    std::jthread daemon_thread_;
    std::jthread request_processor_;
    
    RequestQueueSoA<1024> request_queue_;
    
    AXIOM::FixedVector<std::pair<std::string_view, std::unique_ptr<struct SessionContext>>, 128> sessions_;
    std::mutex sessions_mutex_; 
    
    std::atomic<uint64_t> total_requests_{0};
    std::atomic<uint64_t> rejected_requests_{0};
    std::atomic<double> avg_response_time_{0.0};
    std::atomic<uint32_t> consecutive_failures_{0};
    std::atomic<int64_t> circuit_open_until_ms_{0};
    std::atomic<uint64_t> enqueued_requests_{0};
    std::atomic<uint64_t> completed_requests_{0};
    std::chrono::steady_clock::time_point startup_time_{std::chrono::steady_clock::now()};

#ifdef _WIN32
    HANDLE pipe_handle_{INVALID_HANDLE_VALUE};
#else
    int pipe_fd_{-1};
#endif

public:
    explicit DaemonEngine(std::string_view pipe_name = "axiom_daemon") noexcept;
    ~DaemonEngine() noexcept;

    DaemonEngine(const DaemonEngine&) = delete;
    DaemonEngine& operator=(const DaemonEngine&) = delete;

    [[nodiscard]] std::expected<void, DaemonStatus> start() noexcept;
    void stop() noexcept;
    [[nodiscard]] bool is_running() const noexcept { return running_.load(std::memory_order::seq_cst); }
    [[nodiscard]] DaemonStatus get_status() const noexcept { return status_.load(std::memory_order::seq_cst); }

    [[nodiscard]] Response process_request(const Request& request) noexcept;
    [[nodiscard]] bool send_command(std::string_view session_id, std::string_view command, std::string_view mode = "algebraic") noexcept;

    [[nodiscard]] std::expected<std::string_view, std::error_code> create_session() noexcept;
    [[nodiscard]] bool destroy_session(std::string_view session_id) noexcept;
    void get_active_sessions(FixedVector<std::string_view, 128>& out_sessions) noexcept;

    [[nodiscard]] uint64_t get_total_requests() const noexcept { return total_requests_.load(std::memory_order::seq_cst); }
    [[nodiscard]] uint64_t get_rejected_requests() const noexcept { return rejected_requests_.load(std::memory_order::seq_cst); }
    [[nodiscard]] double get_avg_response_time() const noexcept { return avg_response_time_.load(std::memory_order::seq_cst); }
    [[nodiscard]] std::chrono::milliseconds get_uptime() const noexcept;

private:
    void daemon_loop() noexcept;
    void process_windows_pipe() noexcept;
    void process_posix_pipe() noexcept;
    void request_processor_loop() noexcept;
    [[nodiscard]] std::expected<void, PipeError> setup_pipe() noexcept;
    void cleanup_pipe() noexcept;
    static const char* pipe_error_to_string(PipeError error) noexcept;
    [[nodiscard]] Response execute_command(const Request& request) noexcept;
    void update_metrics(double execution_time) noexcept;
};

struct SessionContext {
    std::array<char, 64> session_id{};
    std::array<char, 32> current_mode{"algebraic"};
    std::chrono::steady_clock::time_point created_at{};
    uint64_t request_count{0};
};

} // namespace AXIOM
