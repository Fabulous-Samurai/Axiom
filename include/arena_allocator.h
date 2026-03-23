/**
 * @file arena_allocator.h
 * @brief AXIOM Engine v3.0 - High-Performance Arena Memory Allocator
 * 
 * Enterprise-grade memory management system:
 * - Zero-fragmentation arena allocation
 * - NUMA-aware memory pools
 * - Cache-line aligned allocations
 * - Memory-mapped file backing
 * - Real-time performance guarantees
 */

#pragma once
#include "fixed_vector.h"

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <thread>
#include <stop_token>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef ENABLE_EIGEN
    #include <Eigen/Dense>
#endif

#include "axiom_export.h"
#include "cpu_optimization.h"

namespace AXIOM {

/**
 * @brief Lightweight atomic spinlock for fast-path lock-free contention
 */
class Spinlock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
    Spinlock() noexcept = default;
    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;
    
    void lock() noexcept {
        while (flag.test_and_set(std::memory_order_acquire)) {
            AXIOM_YIELD_PROCESSOR;
        }
    }
    
    void unlock() noexcept {
        flag.clear(std::memory_order_release);
    }
};

/**
 * @brief Harmonic lock-free arena with background recycling
 *
 * This arena complements MemoryArena for ultra-low-latency burst allocation
 * workloads where deallocation is handled through block rotation.
 */
class AXIOM_EXPORT HarmonicArena {
public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 512ull * 1024ull * 1024ull; // 512MB
    static constexpr size_t SCRIBE_PREPARE_THRESHOLD_PERCENT = 85;

    // Internal block type is kept public so out-of-class member definitions can
    // use it in signatures without static-analysis accessibility warnings.
    struct alignas(AXIOM::CACHE_LINE_SIZE) ArenaBlock {
        struct Deleter {
            size_t cap;
            void operator()(std::byte* p) const noexcept;
        };
        std::unique_ptr<std::byte[], Deleter> storage;
        size_t capacity;
        std::atomic<size_t> offset;
        std::atomic<bool> is_ready;
        std::atomic<ArenaBlock*> next_in_pool;

        explicit ArenaBlock(size_t cap, int node = -1) noexcept;
    };

private:
    alignas(CACHE_LINE_SIZE) std::atomic<ArenaBlock*> current_block_{nullptr};
    alignas(CACHE_LINE_SIZE) std::atomic<ArenaBlock*> spare_block_{nullptr};
    alignas(CACHE_LINE_SIZE) std::atomic<ArenaBlock*> pool_head_{nullptr};

#if defined(__apple_build_version__)
    std::thread maintenance_thread_;
    std::atomic<bool> stop_requested_{false};
#else
    std::jthread maintenance_thread_;
#endif
    size_t block_size_;
    int numa_node_;

public:
    explicit HarmonicArena(size_t block_size = DEFAULT_BLOCK_SIZE, int numa_node = -1);
    ~HarmonicArena();

    HarmonicArena(const HarmonicArena&) = delete;
    HarmonicArena& operator=(const HarmonicArena&) = delete;

    [[nodiscard]] void* allocate(size_t bytes) noexcept;

private:
    [[nodiscard]] static std::byte* reserve_from_block(ArenaBlock* block, size_t bytes) noexcept;
    [[nodiscard]] bool try_install_spare(ArenaBlock* expected_current, ArenaBlock* replacement) noexcept;
    [[nodiscard]] bool should_prepare_spare() const noexcept;

    static size_t align_size(size_t size, size_t alignment) noexcept {
        return (size + alignment - 1) & ~(alignment - 1);
    }

    void push_pool(ArenaBlock* block) noexcept;
    void recycle_spare_block(ArenaBlock* block) noexcept;
    [[nodiscard]] ArenaBlock* pop_pool() noexcept;
    [[nodiscard]] std::byte* switch_and_retry(size_t bytes) noexcept;
#if defined(__apple_build_version__)
    void maintenance_worker() noexcept;
#else
    void maintenance_worker(std::stop_token stop_token) noexcept;
#endif
};

/**
 * @brief High-Performance Memory Arena
 * 
 * Zero-fragmentation allocator for scientific computing workloads
 */
class AXIOM_EXPORT MemoryArena {
public:
    static constexpr size_t DEFAULT_ARENA_SIZE = 64 * 1024 * 1024;  // 64MB
    static constexpr size_t PAGE_SIZE = 4096;
    
    struct ArenaStats {
        size_t total_size;
        size_t used_size;
        size_t free_size;
        size_t peak_usage;
        size_t allocation_count;
        size_t free_count;
        double fragmentation_ratio;
    };

private:
    void* memory_base_{nullptr};
    size_t arena_size_{0};
    std::atomic<size_t> current_offset_{0};
    std::atomic<size_t> peak_usage_{0};
    std::atomic<size_t> allocation_count_{0};
    std::atomic<size_t> free_count_{0};
    
    // Free block management
    struct FreeBlock {
        size_t size;
        FreeBlock* next;
    };
    
    std::atomic<FreeBlock*> free_list_head_{nullptr};
    
    
    // Memory mapping for large allocations
    bool use_memory_mapping_{true};
    
#ifdef _WIN32
    HANDLE file_mapping_{nullptr};
#else
    int backing_fd_{-1};
#endif

public:
    explicit MemoryArena(size_t size = DEFAULT_ARENA_SIZE, bool use_mmap = true);
    ~MemoryArena();
    
    // Core allocation interface
    [[nodiscard]] void* allocate(size_t size, size_t alignment = CACHE_LINE_SIZE) noexcept;
    void deallocate(void* ptr, size_t size) noexcept;
    
    [[nodiscard]] bool is_valid() const noexcept { return memory_base_ != nullptr; }
    
    // Typed allocation helpers
    template<typename T>
    T* allocate_array(size_t count) {
        size_t total_size = count * sizeof(T);
        void* ptr = allocate(total_size, alignof(T));
        return static_cast<T*>(ptr);
    }
    
    template<typename T, typename... Args>
    T* construct(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        return new(ptr) T(std::forward<Args>(args)...);
    }
    
    template<typename T>
    void destroy(T* obj) {
        if (obj) {
            obj->~T();
            deallocate(obj, sizeof(T));
        }
    }
    
    // Arena management
    void reset();
    void trim();
    ArenaStats get_stats() const;
    
    // Memory layout optimization
    void* allocate_aligned_page(size_t size);
    void prefault_pages(void* ptr, size_t size);
    
    // NUMA awareness (Linux only)
#ifdef __linux__
    bool set_numa_policy(int node);
    int get_numa_node() const;
#endif

private:
    bool setup_memory_mapping(size_t size);
    void cleanup_memory_mapping();
    void coalesce_free_blocks();
    size_t align_size(size_t size, size_t alignment) const;
public:
    bool is_pointer_in_arena(void* ptr) const;
};

/**
 * @brief NUMA-Aware Memory Pool Manager
 * 
 * Manages multiple arenas across NUMA nodes for optimal performance
 */
class PoolManager {
public:
    enum class PoolType {
        ZENITH_BANK_A,
        ZENITH_BANK_B
    };

private:
    struct PoolInfo {
        std::unique_ptr<MemoryArena> arena;
        PoolType type{PoolType::ZENITH_BANK_A};
        int numa_node{-1};
        std::atomic<size_t> active_allocations{0};

        PoolInfo() = default;

        PoolInfo(PoolInfo&& other) noexcept
            : arena(std::move(other.arena))
            , type(other.type)
            , numa_node(other.numa_node)
            , active_allocations(other.active_allocations.load()) {}

        ~PoolInfo() = default;
        PoolInfo(const PoolInfo&) = delete;
        PoolInfo& operator=(const PoolInfo&) = delete;
        PoolInfo& operator=(PoolInfo&&) = delete;
    };

    std::array<PoolInfo, 2> pools_;

#ifdef ENABLE_HARMONIC_ARENA
    std::unique_ptr<HarmonicArena> harmonic_arena_;
    std::atomic<size_t> harmonic_allocations_{0};
    static constexpr size_t HARMONIC_FAST_PATH_LIMIT = 1024;
#endif

    thread_local static size_t preferred_pool_index_;

public:
    PoolManager();
    ~PoolManager();

    void* allocate(size_t size, size_t alignment = AXIOM::CACHE_LINE_SIZE);
    void deallocate(void* ptr, size_t size);

    std::vector<MemoryArena::ArenaStats> get_all_stats() const;
    static PoolManager& instance();
};

/**
 * @brief STL-Compatible Arena Allocator
 * 
 * Drop-in replacement for std::allocator using AXIOM memory pools
 */
template<typename T>
class ArenaAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    
    template<typename U>
    struct rebind {
        using other = ArenaAllocator<U>;
    };

private:
    MemoryArena* arena_;

public:
    ArenaAllocator() : arena_(nullptr) {}
    explicit ArenaAllocator(MemoryArena* arena) : arena_(arena) {}
    
    template<typename U>
    ArenaAllocator(const ArenaAllocator<U>& other) : arena_(other.get_arena()) {}
    
    pointer allocate(size_type n) {
        if (arena_) {
            return arena_->allocate_array<T>(n);
        } else {
            return static_cast<pointer>(PoolManager::instance().allocate(n * sizeof(T), alignof(T)));
        }
    }
    
    void deallocate(pointer p, size_type n) {
        if (arena_) {
            arena_->deallocate(p, n * sizeof(T));
        } else {
            PoolManager::instance().deallocate(p, n * sizeof(T));
        }
    }
    
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new(p) U(std::forward<Args>(args)...);
    }
    
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }
    
    MemoryArena* get_arena() const { return arena_; }
    
    bool operator==(const ArenaAllocator& other) const = default;
};

/**
 * @brief High-Performance Vector with Arena Allocation
 * 
 * Optimized vector for numerical computations
 */
// ArenaVector moved to fixed_vector.h

/**
 * @brief Matrix Memory Layout Optimizer
 * 
 * Cache-friendly memory layout for dense matrices
 */
template<typename T>
class OptimizedMatrix {
private:
    ArenaVector<T> data_;
    size_t rows_;
    size_t cols_;
    size_t row_stride_;  // For cache alignment

public:
    OptimizedMatrix(size_t rows, size_t cols, MemoryArena* arena = nullptr)
        : data_(ArenaAllocator<T>(arena))
        , rows_(rows)
        , cols_(cols)
    {
        // Align row stride to cache line boundaries
        row_stride_ = (cols_ * sizeof(T) + AXIOM::CACHE_LINE_SIZE - 1) / AXIOM::CACHE_LINE_SIZE;
        row_stride_ *= AXIOM::CACHE_LINE_SIZE / sizeof(T);
        
        data_.resize(rows_ * row_stride_);
    }
    
    T& operator()(size_t row, size_t col) {
        return data_[row * row_stride_ + col];
    }
    
    const T& operator()(size_t row, size_t col) const {
        return data_[row * row_stride_ + col];
    }
    
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    
    // Direct memory access for BLAS operations
    T* data() { return data_.data(); }
    const T* data() const { return data_.data(); }
    size_t stride() const { return row_stride_; }
};

#ifdef ENABLE_EIGEN
/**
 * @brief Eigen Integration Helper
 * 
 * Custom allocator for Eigen matrices using AXIOM memory pools
 */
namespace EigenIntegration {
    template<typename Scalar, int Options = 0>
    using Matrix = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic, Options>;
    
    template<typename Scalar>
    Matrix<Scalar> create_matrix(size_t rows, size_t cols, MemoryArena* arena = nullptr);
    
    template<typename Scalar>
    class ArenaAlignedAllocator {
    public:
        using value_type = Scalar;
        MemoryArena* arena_;
        
        explicit ArenaAlignedAllocator(MemoryArena* arena = nullptr) : arena_(arena) {}
        
        Scalar* allocate(size_t num_elements) {
            if (arena_) {
                return arena_->allocate_array<Scalar>(num_elements);
            } else {
                return static_cast<Scalar*>(PoolManager::instance().allocate(
                    num_elements * sizeof(Scalar), 32));  // 32-byte alignment for SIMD
            }
        }
        
        void deallocate(Scalar* ptr, size_t num_elements) {
            if (arena_) {
                arena_->deallocate(ptr, num_elements * sizeof(Scalar));
            } else {
                PoolManager::instance().deallocate(ptr);
            }
        }
    };
}
#endif // ENABLE_EIGEN

/**
 * @brief Memory Performance Profiler
 * 
 * Real-time memory performance monitoring and optimization
 */
class MemoryProfiler {
public:
    struct AllocationProfile {
        size_t size;
        size_t alignment;
        std::chrono::high_resolution_clock::time_point timestamp;
        void* address;
        size_t pool_index;
    };
    
    struct PerformanceMetrics {
        double avg_allocation_time_ns;
        double avg_deallocation_time_ns;
        size_t cache_hits;
        size_t cache_misses;
        double memory_efficiency;
        size_t peak_memory_usage;
    };

private:
    std::vector<AllocationProfile> allocation_history_;
    mutable Spinlock history_mutex_;
    std::atomic<bool> profiling_enabled_;

public:
    MemoryProfiler();
    
    void enable_profiling(bool enable = true);
    bool is_profiling_enabled() const;
    
    void record_allocation(void* ptr, size_t size, size_t alignment, size_t pool_index);
    void record_deallocation(void* ptr);
    
    PerformanceMetrics get_metrics() const;
    std::vector<AllocationProfile> get_recent_allocations(size_t count = 100) const;
    
    // Analysis and optimization suggestions
    std::vector<std::string> analyze_patterns() const;
    std::vector<std::string> get_optimization_suggestions() const;
    
    // Global profiler instance
    static MemoryProfiler& instance();
};

} // namespace AXIOM








