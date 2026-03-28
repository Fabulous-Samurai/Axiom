// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file axiom_status.h
 * @brief Central status and error codes for Project AXIOM.
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
    
    // Calculation Errors
    DivideByZero = 1001,
    ParseError = 1002,
    NegativeRoot = 1003,
    DomainError = 1004,
    ArgumentMismatch = 1005,
    NumericOverflow = 1006,
    OperationNotFound = 1007,
    
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

// Legacy alias for compatibility during transition
using CalcErr = StatusCode;

} // namespace AXIOM
