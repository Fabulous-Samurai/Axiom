// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file sentry.cpp
 * @brief System Watchdog Implementation (Operation PLUTO EXODUS)
 */

#include "../include/sentry.h"
#include <iostream>

namespace AXIOM {

Sentry& Sentry::instance() {
    static Sentry sentry_instance;
    return sentry_instance;
}

Sentry::Sentry() : is_running_(false), current_state_(State::NORMAL) {
}

Sentry::~Sentry() {
    stop();
}

bool Sentry::start() {
    if (is_running_.load()) return false;
    is_running_.store(true);

    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    last_core_heartbeat_.store(now);
    last_ui_heartbeat_.store(now);

    monitor_thread_ = std::thread(&Sentry::monitor_loop, this);
    AXIOM_CRASH_MARK(0x5E000001, "Sentry: Watchdog Started");
    return true;
}

void Sentry::stop() {
    is_running_.store(false);
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    AXIOM_CRASH_MARK(0x5E000002, "Sentry: Watchdog Stopped");
}

void Sentry::heartbeat_core() noexcept {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    last_core_heartbeat_.store(now, std::memory_order_relaxed);
}

void Sentry::heartbeat_ui() noexcept {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    last_ui_heartbeat_.store(now, std::memory_order_relaxed);
}

Sentry::State Sentry::get_state() const noexcept {
    return current_state_.load(std::memory_order_acquire);
}

void Sentry::monitor_loop() {
    while (is_running_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Watchdog sleep cycle

        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        auto ui_hb = last_ui_heartbeat_.load(std::memory_order_relaxed);
        auto core_hb = last_core_heartbeat_.load(std::memory_order_relaxed);

        auto ui_delay = now - ui_hb;
        auto core_delay = now - core_hb;

        if (ui_delay > PANIC_THRESHOLD_MS) {
            if (current_state_.load() != State::PANIC) {
                current_state_.store(State::PANIC);
                AXIOM_CRASH_MARK(0xDEADBEEF, "Sentry: UI Thread Deadlock (Panic)");
                trigger_recovery();
            }
        } 
        else if (ui_delay > SUSPICION_THRESHOLD_MS) {
            // UI is lagging, but is the Core busy?
            current_state_.store(State::SUSPICIOUS);
            analyze_situation();
        } 
        else {
            if (current_state_.load() != State::NORMAL) {
                current_state_.store(State::NORMAL);
                AXIOM_CRASH_MARK(0x5E000000, "Sentry: UI Thread Recovered");
            }
        }
    }
}

void Sentry::analyze_situation() {
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    auto core_hb = last_core_heartbeat_.load(std::memory_order_relaxed);
    
    // Sentry (Context Logic)
    if (now - core_hb < 10) {
        // Core is producing heartbeats rapidly, but UI is hanging.
        // Conclusion: Heavy mathematical workload or UI rendering queue is full.
        // Action: Wait. Core is still healthy.
        current_state_.store(State::ANALYZING);
        return; // Don't panic yet.
    } else {
        // Both UI and Core are hung.
        current_state_.store(State::PANIC);
        AXIOM_CRASH_MARK(0xBADF00D, "Sentry: System-wide Freeze Detected");
    }
}

void Sentry::trigger_recovery() {
    // Phase 4: Recovery logic
    // Currently, we just mark the crash. Later we will kill isolated sandboxes.
    std::cerr << "[SENTRY] Triggering Emergency Recovery Protocol!" << std::endl;
}

} // namespace AXIOM
