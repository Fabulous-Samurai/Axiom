// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file cognitive_commander.cpp
 * @brief Implementation of memory pinning and tensor pools for Phase F
 */

#include "cognitive_commander.h"
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
#endif

namespace AXIOM {

PinnedMemoryPool::PinnedMemoryPool(size_t size) noexcept : size_(size) {
#ifdef _WIN32
    base_addr_ = VirtualAlloc(NULL, size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    base_addr_ = mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    std::cout << "[AXIOM Phase F] Allocated SLM Pool: " << size_ << " bytes" << std::endl;
}

PinnedMemoryPool::~PinnedMemoryPool() {
    unpin();
#ifdef _WIN32
    VirtualFree(base_addr_, 0, MEM_RELEASE);
#else
    munmap(base_addr_, size_);
#endif
}

bool PinnedMemoryPool::pin() noexcept {
    if (!base_addr_) return false;
#ifdef _WIN32
    if (VirtualLock(base_addr_, size_)) {
        std::cout << "[AXIOM Phase F] Weights pinned successfully (VirtualLock)" << std::endl;
        return true;
    }
#else
    if (mlock(base_addr_, size_) == 0) {
        std::cout << "[AXIOM Phase F] Weights pinned successfully (mlock)" << std::endl;
        return true;
    }
#endif
    std::cerr << "[AXIOM Phase F] PINNING FAILURE - System may swap model weights!" << std::endl;
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
    size_t next = (current + size + 63) & ~63; // Cache-line alignment
    if (next > size_) return nullptr;

    while (!offset_.compare_exchange_weak(current, next, std::memory_order_release, std::memory_order_relaxed)) {
        next = (current + size + 63) & ~63;
        if (next > size_) return nullptr;
    }

    return static_cast<char*>(base_addr_) + current;
}

bool CognitiveCommander::load_weights(const std::string& model_path) {
    // Phase F Placeholder: In a real SLM integration, this would load weights from disk
    // directly into weight_pool_ using zero-copy DMA or mmap.
    weight_pool_ = std::make_unique<PinnedMemoryPool>(512 * 1024 * 1024); // 512MB weight pool
    if (!weight_pool_->pin()) return false;
    
    activation_pool_ = std::make_unique<PinnedMemoryPool>(128 * 1024 * 1024); // 128MB activation pool
    return true;
}

void CognitiveCommander::run_inference(const std::vector<float>& input, std::vector<float>& output) {
    // Inference happens here using the activation_pool_ for intermediate tensors
    // This ensures ZERO heap allocations during decision cycles.
}

} // namespace AXIOM
