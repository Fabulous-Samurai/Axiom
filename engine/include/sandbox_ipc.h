// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file sandbox_ipc.h
 * @brief Lock-Free Shared Memory IPC for Axiom Sandbox.
 */

#pragma once

#include "lock_free_ring_buffer.h"
#include <atomic>
#include <cstdint>

namespace AXIOM {
namespace Sandbox {

static constexpr size_t MAX_CMD_LEN = 512;
static constexpr size_t MAX_RES_LEN = 2048;

struct SandboxRequest {
    uint64_t request_id;
    char command[MAX_CMD_LEN];
};

struct SandboxResponse {
    uint64_t request_id;
    bool success;
    char result[MAX_RES_LEN];
    char error[256];
    double execution_time_ms;
};

/**
 * @brief Memory layout for the shared IPC segment.
 * This structure lives directly in the mmap'd region.
 */
struct alignas(64) SandboxIPCLayout {
    // Magic signature to verify segment integrity
    uint64_t magic_signature = 0x4158494F4D534258; // "AXIOMSBX"

    // Heartbeats for Sentry/Watchdog
    std::atomic<uint64_t> worker_heartbeat{0};
    std::atomic<uint32_t> worker_pid{0};

    // Lock-Free Queues
    // Main process pushes to req_queue, Worker pops.
    SPSCQueue<SandboxRequest, 128> req_queue;
    
    // Worker pushes to res_queue, Main process pops.
    SPSCQueue<SandboxResponse, 128> res_queue;
};

} // namespace Sandbox
} // namespace AXIOM
