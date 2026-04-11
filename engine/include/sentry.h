// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file sentry.h
 * @brief Phase 4: Sentry & Watchdog - High-Availability Nano-Service Guardian
 *
 * Sentry monitors the isolated core (Pluto/Mantis) and the UI thread via
 * atomic heartbeats. If the UI freezes, Sentry prevents the core from blocking.
 * If a Sandbox times out, Sentry terminates it safely.
 */

#pragma once

#include "crash_dump.h"
#include <atomic>
#include <thread>
#include <chrono>

namespace AXIOM {

/**
 * @brief The High-Availability Sentinel.
 */
class Sentry {
public:
    enum class State {
        NORMAL = 0,
        SUSPICIOUS = 1,     // Watchdog is barking (Delay detected)
        ANALYZING = 2,      // Sentry checking context (Is it a heavy workload or a freeze?)
        RECOVERING = 3,     // Killing sandbox or shedding load
        PANIC = 4           // Unrecoverable state, dumping flight data
    };

    Sentry();
    ~Sentry();

    /**
     * @brief Starts the isolated Sentry monitoring thread.
     */
    bool start();

    /**
     * @brief Stops the Sentry thread gracefully.
     */
    void stop();

    /**
     * @brief Called by the hot-loop (Isolated Core) to signal it is alive and working.
     */
    void heartbeat_core() noexcept;

    /**
     * @brief Called by the UI thread to signal it is responsive.
     */
    void heartbeat_ui() noexcept;

    /**
     * @brief Returns the current state of the system.
     */
    State get_state() const noexcept;

    /**
     * @brief Singleton instance.
     */
    static Sentry& instance();

private:
    void monitor_loop();
    void analyze_situation();
    void trigger_recovery();

    std::atomic<uint64_t> last_core_heartbeat_{0};
    std::atomic<uint64_t> last_ui_heartbeat_{0};
    std::atomic<State>    current_state_{State::NORMAL};
    std::atomic<bool>     is_running_{false};

    std::jthread monitor_thread_;

    // Thresholds
    static constexpr uint64_t SUSPICION_THRESHOLD_MS = 50;  // 50ms without UI heartbeat = Suspicious
    static constexpr uint64_t PANIC_THRESHOLD_MS = 2000;    // 2s without UI heartbeat = Panic
};

} // namespace AXIOM
