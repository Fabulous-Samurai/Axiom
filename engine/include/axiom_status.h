// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file axiom_status.h
 * @brief Central status, error codes, and orchestration types for Project AXIOM.
 */

#pragma once

#include <stdint.h>

namespace AXIOM {

/**
 * @brief Universal calculation and engine error codes.
 * Replaces and extends the legacy CalcErr enum.
 */
enum class StatusCode : int32_t {
    Success = 0,
    None = 0,
    
    // Calculation Errors
    DivideByZero = 1001,
    ParseError = 1002,
    NegativeRoot = 1003,
    DomainError = 1004,
    ArgumentMismatch = 1005,
    NumericOverflow = 1006,
    OperationNotFound = 1007,
    IndeterminateResult = 1008,
    StackOverflow = 1009,
    InfiniteLoop = 1010,
    
    // Resource Errors
    MemoryExhausted = 2001,
    ResourceLocked = 2002,
    Timeout = 2003,
    
    // IPC/Pipe Errors
    PipeError = 3001,
    PermissionDenied = 3002,
    InvalidName = 3003,
    
    // Security Errors
    VaultLocked = 4001,
    AccessDenied = 4002,
    IntegrityCheckFailed = 4003,
    
    Unknown = 9999
};

// Legacy alias for compatibility
using CalcErr = StatusCode;

enum class LinAlgErr : int32_t {
    None = 0,
    NoSolution = 1,
    InfiniteSolutions = 2,
    MatrixMismatch = 3,
    ParseError = 4
};

enum class ComputeEngine { Native, Eigen, Python, Vulkan, Auto };
enum class OperationComplexity { Simple, Medium, Complex, VeryComplex };

} // namespace AXIOM
