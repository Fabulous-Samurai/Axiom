// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file sandbox_proxy.cpp
 * @brief Out-of-process isolation proxy implementation (Operation PLUTO EXODUS)
 */

#include "sandbox_proxy.h"
#include "sandbox_ipc.h"
#include "sentry.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <new>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

namespace AXIOM::Sandbox {

static const char* SHM_NAME = "Local\\AXIOM_SANDBOX_SHM";
static const size_t SHM_SIZE = sizeof(SandboxIPCLayout);

ProcessProxy& ProcessProxy::instance() {
    static ProcessProxy proxy_instance;
    return proxy_instance;
}

ProcessProxy::ProcessProxy() : is_worker_alive_(false) {
#ifdef _WIN32
    h_map_file_ = CreateFileMappingA(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (DWORD)SHM_SIZE, SHM_NAME);
    
    if (h_map_file_) {
        ipc_layout_ = (SandboxIPCLayout*)MapViewOfFile(
            h_map_file_, FILE_MAP_ALL_ACCESS, 0, 0, SHM_SIZE);
        
        if (ipc_layout_) {
            // Placement new for the layout (to initialize atomic counters/queues)
            new(ipc_layout_) SandboxIPCLayout();
        }
    }
#else
    // POSIX shm_open implementation placeholder
#endif
}

ProcessProxy::~ProcessProxy() {
    terminate_worker();
#ifdef _WIN32
    if (ipc_layout_) UnmapViewOfFile(ipc_layout_);
    if (h_map_file_) CloseHandle(h_map_file_);
#endif
}

bool ProcessProxy::spawn_worker() {
    if (is_worker_alive_.load()) return true;

#ifdef _WIN32
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    wchar_t cmd[] = L"axiom_worker.exe --ipc-shm AXIOM_SANDBOX_SHM";

    if (CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        h_process_ = pi.hProcess;
        h_thread_ = pi.hThread;
        is_worker_alive_.store(true);
        if (ipc_layout_) ipc_layout_->worker_pid.store((uint32_t)pi.dwProcessId);
        return true;
    }
    return false;
#else
    // fork/exec placeholder
    return false;
#endif
}

std::future<SandboxResult> ProcessProxy::execute(const std::string& command, std::chrono::milliseconds timeout_ms) {
    return std::async(std::launch::async, [this, command, timeout_ms]() -> SandboxResult {
        if (!is_worker_alive_.load() || !ipc_layout_) {
            if (!spawn_worker()) {
                return {false, "", "Sandbox Error: Failed to spawn worker", 0.0};
            }
        }

        uint64_t req_id = std::chrono::steady_clock::now().time_since_epoch().count();
        SandboxRequest req;
        req.request_id = req_id;
        std::snprintf(req.command, sizeof(req.command), "%s", command.c_str());

        // Push to Lock-Free Queue
        if (!ipc_layout_->req_queue.push(req)) {
            return {false, "", "Sandbox Error: Request queue full", 0.0};
        }

        // Wait for result (Polling for performance, with timeout)
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < timeout_ms) {
            SandboxResponse res;
            if (ipc_layout_->res_queue.pop(res)) {
                if (res.request_id == req_id) {
                    return {res.success, std::string(res.result), std::string(res.error), res.execution_time_ms};
                }
            }
            std::this_thread::yield(); // OS-bypass spin-wait
        }

        // --- SELF-HEALING: KILL THE ZOMBIE WORKER ---
        terminate_worker(); 
        spawn_worker(); // Reboot the quarantine zone

        return {false, "", "Sandbox Error: Timeout - Stalled Process Terminated", 0.0};
    });
}

void ProcessProxy::terminate_worker() {
    if (!is_worker_alive_.load()) return;
#ifdef _WIN32
    if (h_process_) {
        TerminateProcess(h_process_, 0);
        CloseHandle(h_process_);
        CloseHandle(h_thread_);
        h_process_ = nullptr;
    }
#endif
    is_worker_alive_.store(false);
}

} // namespace AXIOM::Sandbox
