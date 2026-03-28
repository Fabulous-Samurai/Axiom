// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "axiom_export.h"

/**
 * @file sleef_wrapper.h
 * @brief C-linkable wrappers for Sleef math functions to support JIT inlining.
 */

extern "C" {

AXIOM_EXPORT void axiom_sleef_sin_d4(const double* in, double* out);
AXIOM_EXPORT void axiom_sleef_cos_d4(const double* in, double* out);
AXIOM_EXPORT void axiom_sleef_tan_d4(const double* in, double* out);
AXIOM_EXPORT void axiom_sleef_log_d4(const double* in, double* out);
AXIOM_EXPORT void axiom_sleef_log10_d4(const double* in, double* out);
AXIOM_EXPORT void axiom_sleef_exp_d4(const double* in, double* out);

AXIOM_EXPORT double axiom_sleef_sin_d1(double in);
AXIOM_EXPORT double axiom_sleef_cos_d1(double in);
AXIOM_EXPORT double axiom_sleef_tan_d1(double in);
AXIOM_EXPORT double axiom_sleef_log_d1(double in);
AXIOM_EXPORT double axiom_sleef_log10_d1(double in);
AXIOM_EXPORT double axiom_sleef_exp_d1(double in);

} // extern "C"
