// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "../include/telemetry.h"
#include <iostream>
#include <format>

namespace AXIOM {

TelemetryScribe& TelemetryScribe::instance() {
    static TelemetryScribe inst;
    return inst;
}

bool TelemetryScribe::start(std::string_view log_path) {
    std::cout << "[AXIOM Telemetry] Starting Phase G (Heisenberg-Defying) scribe for: " << log_path << std::endl;
    if (running_.load(std::memory_order_acquire)) return false;

    // [MANDATORY PATH]: Initialize HW Counters before hot-path entry
    if (!PMUOrchestrator::instance().Initialize()) {
        std::cerr << "[AXIOM Telemetry] WARNING: PMU Initialization failed. Falling back to RDTSC-only mode." << std::endl;
    }
    
    log_file_path_ = std::string(log_path);
    running_.store(true, std::memory_order_release);
    
    scribe_thread_ = std::jthread([this] { scribe_loop(); });
    return true;
}

void TelemetryScribe::shutdown() {
    if (!running_.load(std::memory_order_acquire)) return;
    
    running_.store(false, std::memory_order_release);
    if (scribe_thread_.joinable()) {
        scribe_thread_.join();
    }
}

void TelemetryScribe::scribe_loop() {
    std::ofstream log_file(log_file_path_, std::ios::binary | std::ios::out);
    if (!log_file.is_open()) {
        std::cerr << "[AXIOM Telemetry] Failed to open log file: " << log_file_path_ << std::endl;
        running_.store(false, std::memory_order_release);
        return;
    }

    // Write header
    uint32_t magic = 0xABCD1234;
    log_file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));

    while (running_.load(std::memory_order_acquire) || head_.load(std::memory_order_acquire) != tail_.load(std::memory_order_acquire)) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t current_head = head_.load(std::memory_order_acquire);

        if (current_tail == current_head) {
            std::this_thread::yield();
            continue;
        }

        // Process batch of records
        while (current_tail != current_head) {
            const TelemetryRecord& rec = ring_buffer_[current_tail];
            log_file.write(reinterpret_cast<const char*>(&rec), sizeof(TelemetryRecord));
            
            current_tail = (current_tail + 1) % RING_BUFFER_SIZE;
            tail_.store(current_tail, std::memory_order_release);
        }
        
        log_file.flush();
    }
    
    log_file.close();
}

} // namespace AXIOM

