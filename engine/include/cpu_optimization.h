// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file cpu_optimization.h
 * @brief CPU-specific optimizations and performance tuning
 */

#pragma once

#ifndef CPU_OPTIMIZATION_H
#define CPU_OPTIMIZATION_H

#include <memory>
#include <string>

#if defined(_MSC_VER)
#include <immintrin.h>
#include <intrin.h>
#define AXIOM_FORCE_INLINE __forceinline
#define AXIOM_NO_INLINE __declspec(noinline)
#define AXIOM_HOT
#define AXIOM_RDTSC() __rdtsc()
#define AXIOM_YIELD_PROCESSOR() _mm_pause()
#define AXIOM_LFENCE() _mm_lfence()
#elif defined(__GNUC__) || defined(__clang__)
#define AXIOM_FORCE_INLINE inline __attribute__((always_inline))
#define AXIOM_NO_INLINE __attribute__((noinline))
#define AXIOM_HOT __attribute__((hot))
#if defined(__i386__) || defined(__x86_64__)
#include <immintrin.h>
#define AXIOM_RDTSC()                               \
  ({                                                \
    uint32_t lo, hi;                                \
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi)); \
    ((uint64_t)hi << 32) | lo;                      \
  })
#define AXIOM_YIELD_PROCESSOR() __builtin_ia32_pause()
#define AXIOM_LFENCE() __asm__ volatile("lfence" ::: "memory")
#elif defined(__aarch64__)
#include <arm_neon.h>
#define AXIOM_RDTSC()                                   \
  ({                                                    \
    uint64_t val;                                       \
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(val)); \
    val;                                                \
  })
#define AXIOM_YIELD_PROCESSOR() __asm__ volatile("yield" ::: "memory")
#define AXIOM_LFENCE() __asm__ volatile("isb sy" ::: "memory")
#else
#define AXIOM_RDTSC() 0ULL
#define AXIOM_YIELD_PROCESSOR() ((void)0)
#define AXIOM_LFENCE() __asm__ volatile("" ::: "memory")
#endif
#else
#define AXIOM_FORCE_INLINE inline
#define AXIOM_NO_INLINE
#define AXIOM_HOT
#define AXIOM_YIELD_PROCESSOR() ((void)0)
#define AXIOM_RDTSC() 0ULL
#endif

namespace AXIOM {

constexpr size_t CACHE_LINE_SIZE = 64;
// ============================================================================
// Alignment for AVX 256-bit vectors (32 bytes)
// ============================================================================
#define AXIOM_ALIGN_AVX alignas(32)
#define AXIOM_ALIGN_CACHE alignas(CACHE_LINE_SIZE)

// ============================================================================
// Function target attribute for AVX2+FMA multi-versioning
// ============================================================================
#if defined(__GNUC__) || defined(__clang__)
#define AXIOM_TARGET_AVX2_FMA __attribute__((target("avx2,fma")))
#else
#define AXIOM_TARGET_AVX2_FMA
#endif

// ============================================================================
// Compile-time CPU Feature Guard
// Reads CMake-propagated defines (AXIOM_SIMD_*_ENABLED) to determine
// which instruction sets are available at build time.
// ============================================================================
struct CpuFeatureGuard {
  static constexpr bool has_avx2 =
#ifdef AXIOM_SIMD_AVX2_ENABLED
      true;
#else
      false;
#endif

  static constexpr bool has_fma =
#ifdef AXIOM_SIMD_FMA_ENABLED
      true;
#else
      false;
#endif

  static constexpr bool has_avx_vnni =
#ifdef AXIOM_SIMD_AVX_VNNI_ENABLED
      true;
#else
      false;
#endif

  static constexpr bool has_avx512 =
#ifdef AXIOM_SIMD_AVX512_ENABLED
      true;
#else
      false;
#endif

  static constexpr bool has_neon =
#if defined(__aarch64__) || defined(__ARM_NEON)
      true;
#else
      false;
#endif

  /// True when both AVX2 and FMA are available (FMA3 dot-product kernel)
  static constexpr bool can_fma3_dot = has_avx2 && has_fma;

  /// True when VNNI is available (INT8 dot-product kernel)
  static constexpr bool can_vnni_dot = has_avx_vnni;
};

/**
 * @brief CPU optimization utilities
 */
class CPUOptimization {
 public:
  static void Initialize();
  static std::string GetCPUInfo();
  static void OptimizeForCurrentCPU();

  /**
   * @brief Set thread affinity for the calling thread.
   * @param core_index The CPU core index to pin to.
   */
  static void SetThreadAffinity(int core_index) noexcept;

 private:
  static bool DetectSSE();
  static bool DetectAVX();
  static bool DetectAVX2();
};

}  // namespace AXIOM

#endif  // CPU_OPTIMIZATION_H
