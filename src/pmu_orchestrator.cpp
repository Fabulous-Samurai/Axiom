/**
 * @file pmu_orchestrator.cpp
 * @brief PMU Orchestration implementation using perf_event_open + mmap bridge.
 */

#include "../include/pmu_orchestrator.h"
#include <iostream>

#if defined(__linux__)
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>

static long perf_event_open(struct perf_event_attr* hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}
#endif

namespace AXIOM {

PMUOrchestrator& PMUOrchestrator::instance() {
    static PMUOrchestrator inst;
    return inst;
}

bool PMUOrchestrator::Initialize() noexcept {
    if (initialized_) return true;

#if defined(__linux__)
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.size = sizeof(struct perf_event_attr);
    pe.disabled = 0;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    uint32_t configs[] = {
        PERF_COUNT_HW_INSTRUCTIONS,
        PERF_COUNT_HW_CPU_CYCLES,
        PERF_COUNT_HW_CACHE_MISSES, // Note: Simplified, usually need PERF_COUNT_HW_CACHE_L1D
        PERF_COUNT_HW_BRANCH_MISSES
    };

    for (int i = 0; i < 4; ++i) {
        pe.type = PERF_TYPE_HARDWARE;
        pe.config = configs[i];
        
        int fd = perf_event_open(&pe, 0, -1, -1, 0);
        if (fd == -1) {
            std::cerr << "[PMU] Failed to open perf_event " << i << std::endl;
            return false;
        }

        // mmap the metadata page to authorize rdpmc in userspace
        void* addr = mmap(NULL, getpagesize(), PROT_READ, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            close(fd);
            return false;
        }

        pages_[i].fd = fd;
        pages_[i].mmap_base = addr;
    }
    initialized_ = true;
    return true;
#else
    // Windows/Other: Unsupported for direct rdpmc via perf_event_open
    return false; 
#endif
}

void PMUOrchestrator::Shutdown() noexcept {
    if (!initialized_) return;

#if defined(__linux__)
    for (int i = 0; i < 4; ++i) {
        if (pages_[i].mmap_base) munmap(pages_[i].mmap_base, getpagesize());
        if (pages_[i].fd != -1) close(pages_[i].fd);
    }
#endif
    initialized_ = false;
}

uint64_t PMUOrchestrator::ReadCounter(int idx) const noexcept {
#if defined(__linux__)
    if (!initialized_) return 0;
    
    // [MANDATORY PATH]: Read metadata page to get hardware index
    auto* pc = static_cast<struct perf_event_mmap_page*>(pages_[idx].mmap_base);
    uint32_t index = pc->index;

    if (index == 0) return 0; // Hardware counter not available

    // [MANDATORY PATH]: AXIOM_LFENCE serialization BEFORE rdpmc
    AXIOM_LFENCE;
    
    // Index-1 because rdpmc uses 0-based indexing for hardware counters
    // but perf_event_mmap_page index is 1-based (0 is disabled).
    return (uint64_t)__builtin_ia32_rdpmc(index - 1);
#else
    return 0;
#endif
}

} // namespace AXIOM

