/**
 * @file execution_orchestrator.cpp
 * @brief Implementation of the Unified ExecutionOrchestrator.
 * 
 * Synchronized with Zenith-Pure core types (FixedVector, string_view).
 */

#include "execution_orchestrator.h"
#include "dynamic_calc.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>

namespace AXIOM {

struct ExecutionOrchestrator::Impl {
    ComputeEngine preferred_engine_{ComputeEngine::Auto};
    bool fallback_enabled_{true};
    
    struct EngineStatus { ComputeEngine engine; bool available; };
    AXIOM::FixedVector<EngineStatus, 8> engine_availability_;
    
    struct {
        std::string_view last_op;
        ComputeEngine engine;
        double time_ms;
    } metrics_;

    Impl() {
        engine_availability_.push_back({ComputeEngine::Native, true});
        engine_availability_.push_back({ComputeEngine::Vulkan, false}); // To be enabled in Phase 7
    }

    bool IsAvailable(ComputeEngine engine) const {
        for (const auto& status : engine_availability_) {
            if (status.engine == engine) return status.available;
        }
        return false;
    }
};

ExecutionOrchestrator::ExecutionOrchestrator() : pimpl_(std::make_unique<Impl>()) {}
ExecutionOrchestrator::~ExecutionOrchestrator() = default;

void ExecutionOrchestrator::SetPreferredEngine(ComputeEngine engine) {
    pimpl_->preferred_engine_ = engine;
}

void ExecutionOrchestrator::EnableFallback(bool enable) {
    pimpl_->fallback_enabled_ = enable;
}

EngineResult ExecutionOrchestrator::DispatchOperation(std::string_view operation,
                                                   const AXIOM::FixedVector<std::string_view, 16>& args,
                                                   ComputeEngine preferred_engine) {
    const auto start = std::chrono::high_resolution_clock::now();
    
    ComputeEngine engine = (preferred_engine == ComputeEngine::Auto)
        ? pimpl_->preferred_engine_ : preferred_engine;

    if (!pimpl_->IsAvailable(engine) || engine == ComputeEngine::Auto) {
        engine = ComputeEngine::Native;
    }

    // Build operation string in a fixed buffer (Zero-Allocation)
    char buffer[2048];
    size_t offset = 0;
    auto append = [&](std::string_view sv) {
        if (offset + sv.size() < sizeof(buffer)) {
            std::memcpy(buffer + offset, sv.data(), sv.size());
            offset += sv.size();
        }
    };

    append(operation);
    for (const auto& arg : args) {
        append(" ");
        append(arg);
    }
    
    std::string_view full_expr(buffer, offset);
    
    thread_local DynamicCalc native;
    auto result = native.Evaluate(full_expr);

    auto end = std::chrono::high_resolution_clock::now();
    pimpl_->metrics_.time_ms = std::chrono::duration<double, std::milli>(end - start).count();
    pimpl_->metrics_.engine = engine;
    pimpl_->metrics_.last_op = operation;

    return result;
}

EngineResult ExecutionOrchestrator::DispatchMatrixOperation(std::string_view operation,
                                                         const Eigen::Ref<const Eigen::MatrixXd>& matrix_data) {
    // Optimization: In Phase 7, this will directly map the Eigen matrix to a Vulkan SSBO.
    // For now, we fallback to Native evaluation.
    return CreateErrorResult(CalcErr::OperationNotFound);
}

uint64_t ExecutionOrchestrator::OffloadToGPU(const ComputeTask& task, const void* data, size_t size) {
    if (!pimpl_->IsAvailable(ComputeEngine::Vulkan)) return 0;
    
    // [PHASE 7]: Initiate Vulkan Compute Pipeline
    // 1. Acquire SSBO from Pool
    // 2. Map data (Zero-Copy)
    // 3. Dispatch Shader
    // 4. Return Task ID for future synchronization
    return 12345; // Placeholder task ID
}

std::string_view ExecutionOrchestrator::GetPerformanceReport() const {
    static char report_buffer[512];
    int len = std::snprintf(report_buffer, sizeof(report_buffer),
        "Last: %.*s | Engine: %d | Time: %.4fms",
        (int)pimpl_->metrics_.last_op.size(), pimpl_->metrics_.last_op.data(),
        (int)pimpl_->metrics_.engine,
        pimpl_->metrics_.time_ms);
    return std::string_view(report_buffer, len > 0 ? len : 0);
}

} // namespace AXIOM
