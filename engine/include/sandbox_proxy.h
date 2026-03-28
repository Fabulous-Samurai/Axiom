// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file sandbox_proxy.h
 * @brief Out-of-Process Sandbox Proxy for heavy calculations.
 * 
 * Redirects "Complex" marked tasks to an isolated axiom_worker.exe process
 * to prevent main engine thread blocking or crashes.
 */

#pragma once

#include <string>
#include <vector>
#include <future>
#include <atomic>
#include <chrono>

namespace AXIOM {
namespace Sandbox {

struct SandboxResult {
    bool success = false;
    std::string result;
    std::string error;
    double execution_time_ms = 0.0;
};

/**
 * @brief Proxy that manages the lifecycle of a worker process.
 */
class ProcessProxy {
public:
    ProcessProxy();
    ~ProcessProxy();

    /**
     * @brief Executes a command in an isolated sandbox process.
     * @param command The mathematical expression or command string.
     * @param timeout_ms Maximum time allowed before Sentry kills the worker.
     */
    std::future<SandboxResult> execute(const std::string& command, 
                                       std::chrono::milliseconds timeout_ms = std::chrono::milliseconds(500));

    /**
     * @brief Singleton instance for global sandbox access.
     */
    static ProcessProxy& instance();

private:
    bool spawn_worker();
    void monitor_worker();
    void terminate_worker();

#ifdef _WIN32
    void* h_process_{nullptr};
    void* h_thread_{nullptr};
    void* h_map_file_{nullptr};
#else
    int worker_pid_{-1};
#endif

    struct SandboxIPCLayout* ipc_layout_{nullptr};
    std::atomic<bool> is_worker_alive_{false};
    
    // Future expansion: IPC handles (Shared Memory / Pipes)
};

} // namespace Sandbox
} // namespace AXIOM
