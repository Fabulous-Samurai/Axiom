// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
#include "sleef_wrapper.h"

#include <cmath>
#include <cstring>

#define SLEEF_STATIC_LIBS
#include <sleef.h>

extern "C" {

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>

void axiom_sleef_sin_d4(const double* in, double* out) {
  __m256d vin = _mm256_loadu_pd(in);
  __m256d vout = Sleef_sind4_u10avx2(vin);
  _mm256_storeu_pd(out, vout);
}

void axiom_sleef_cos_d4(const double* in, double* out) {
  __m256d vin = _mm256_loadu_pd(in);
  __m256d vout = Sleef_cosd4_u10avx2(vin);
  _mm256_storeu_pd(out, vout);
}

void axiom_sleef_tan_d4(const double* in, double* out) {
  __m256d vin = _mm256_loadu_pd(in);
  __m256d vout = Sleef_tand4_u10avx2(vin);
  _mm256_storeu_pd(out, vout);
}

void axiom_sleef_log_d4(const double* in, double* out) {
  __m256d vin = _mm256_loadu_pd(in);
  __m256d vout = Sleef_logd4_u10avx2(vin);
  _mm256_storeu_pd(out, vout);
}

void axiom_sleef_log10_d4(const double* in, double* out) {
  __m256d vin = _mm256_loadu_pd(in);
  __m256d vout = Sleef_log10d4_u10avx2(vin);
  _mm256_storeu_pd(out, vout);
}

void axiom_sleef_exp_d4(const double* in, double* out) {
  __m256d vin = _mm256_loadu_pd(in);
  __m256d vout = Sleef_expd4_u10avx2(vin);
  _mm256_storeu_pd(out, vout);
}

#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>

// Sleef_sind4_u10advsimd typically uses a custom struct to hold 4 doubles
// (2x128-bit)
void axiom_sleef_sin_d4(const double* in, double* out) {
  float64x2x2_t vin;
  vin.val[0] = vld1q_f64(in);
  vin.val[1] = vld1q_f64(in + 2);
  float64x2x2_t vout = Sleef_sind4_u10advsimd(vin);
  vst1q_f64(out, vout.val[0]);
  vst1q_f64(out + 2, vout.val[1]);
}

void axiom_sleef_cos_d4(const double* in, double* out) {
  float64x2x2_t vin;
  vin.val[0] = vld1q_f64(in);
  vin.val[1] = vld1q_f64(in + 2);
  float64x2x2_t vout = Sleef_cosd4_u10advsimd(vin);
  vst1q_f64(out, vout.val[0]);
  vst1q_f64(out + 2, vout.val[1]);
}

void axiom_sleef_tan_d4(const double* in, double* out) {
  float64x2x2_t vin;
  vin.val[0] = vld1q_f64(in);
  vin.val[1] = vld1q_f64(in + 2);
  float64x2x2_t vout = Sleef_tand4_u10advsimd(vin);
  vst1q_f64(out, vout.val[0]);
  vst1q_f64(out + 2, vout.val[1]);
}

void axiom_sleef_log_d4(const double* in, double* out) {
  float64x2x2_t vin;
  vin.val[0] = vld1q_f64(in);
  vin.val[1] = vld1q_f64(in + 2);
  float64x2x2_t vout = Sleef_logd4_u10advsimd(vin);
  vst1q_f64(out, vout.val[0]);
  vst1q_f64(out + 2, vout.val[1]);
}

void axiom_sleef_log10_d4(const double* in, double* out) {
  float64x2x2_t vin;
  vin.val[0] = vld1q_f64(in);
  vin.val[1] = vld1q_f64(in + 2);
  float64x2x2_t vout = Sleef_log10d4_u10advsimd(vin);
  vst1q_f64(out, vout.val[0]);
  vst1q_f64(out + 2, vout.val[1]);
}

void axiom_sleef_exp_d4(const double* in, double* out) {
  float64x2x2_t vin;
  vin.val[0] = vld1q_f64(in);
  vin.val[1] = vld1q_f64(in + 2);
  float64x2x2_t vout = Sleef_expd4_u10advsimd(vin);
  vst1q_f64(out, vout.val[0]);
  vst1q_f64(out + 2, vout.val[1]);
}

#else
// Fallback for other architectures
void axiom_sleef_sin_d4(const double* in, double* out) {
  for (int i = 0; i < 4; ++i) out[i] = std::sin(in[i]);
}
void axiom_sleef_cos_d4(const double* in, double* out) {
  for (int i = 0; i < 4; ++i) out[i] = std::cos(in[i]);
}
void axiom_sleef_tan_d4(const double* in, double* out) {
  for (int i = 0; i < 4; ++i) out[i] = std::tan(in[i]);
}
void axiom_sleef_log_d4(const double* in, double* out) {
  for (int i = 0; i < 4; ++i) out[i] = std::log(in[i]);
}
void axiom_sleef_log10_d4(const double* in, double* out) {
  for (int i = 0; i < 4; ++i) out[i] = std::log10(in[i]);
}
void axiom_sleef_exp_d4(const double* in, double* out) {
  for (int i = 0; i < 4; ++i) out[i] = std::exp(in[i]);
}
#endif

double axiom_sleef_sin_d1(double in) { return std::sin(in); }
double axiom_sleef_cos_d1(double in) { return std::cos(in); }
double axiom_sleef_tan_d1(double in) { return std::tan(in); }
double axiom_sleef_log_d1(double in) { return std::log(in); }
double axiom_sleef_log10_d1(double in) { return std::log10(in); }
double axiom_sleef_exp_d1(double in) { return std::exp(in); }

}  // extern "C"
