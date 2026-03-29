// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file memory_pool.cpp
 * @brief Implementation of hardware-pinned memory allocation (Operation DRY-POOL).
 */

#include "memory_pool.h"
#include <iostream>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #include <memoryapi.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

namespace AXIOM {

PinnedMemoryPool::PinnedMemoryPool(size_t size) noexcept : size_(size) {
#ifdef _WIN32
    base_addr_ = VirtualAlloc(NULL, size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    base_addr_ = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base_addr_ == MAP_FAILED) base_addr_ = nullptr;
#endif
    if (base_addr_) {
        std::cout << "[AXIOM MEMORY] SLM Pool Initialized: " << size_ << " bytes." << std::endl;
    }
}

PinnedMemoryPool::~PinnedMemoryPool() {
    unpin();
#ifdef _WIN32
    if (base_addr_) VirtualFree(base_addr_, 0, MEM_RELEASE);
#else
    if (base_addr_) munmap(base_addr_, size_);
#endif
}

bool PinnedMemoryPool::pin() noexcept {
    if (!base_addr_) return false;
#ifdef _WIN32
    if (VirtualLock(base_addr_, size_)) {
        std::cout << "[AXIOM MEMORY] Region pinned to RAM (VirtualLock)." << std::endl;
        return true;
    }
#else
    if (mlock(base_addr_, size_) == 0) {
        std::cout << "[AXIOM MEMORY] Region pinned to RAM (mlock)." << std::endl;
        return true;
    }
#endif
    std::cerr << "[AXIOM MEMORY] PINNING FAILURE - Kernel may swap this region!" << std::endl;
    return false;
}

void PinnedMemoryPool::unpin() noexcept {
    if (!base_addr_) return;
#ifdef _WIN32
    VirtualUnlock(base_addr_, size_);
#else
    munlock(base_addr_, size_);
#endif
}

void* PinnedMemoryPool::allocate(size_t size) noexcept {
    size_t current = offset_.load(std::memory_order_relaxed);
    size_t next = (current + size + 63) & ~63; // 64-byte Cache-line alignment
    
    if (next > size_) return nullptr;

    while (!offset_.compare_exchange_weak(current, next, std::memory_order_release, std::memory_order_relaxed)) {
        next = (current + size + 63) & ~63;
        if (next > size_) return nullptr;
    }

    return static_cast<char*>(base_addr_) + current;
}

} // namespace AXIOM
