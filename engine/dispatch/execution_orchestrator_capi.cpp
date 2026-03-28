/**
 * @file execution_orchestrator_capi.cpp
 * @brief AXIOM FFI Boundary Implementation for ExecutionOrchestrator
 */

#include "execution_orchestrator_capi.h"
#include "execution_orchestrator.h"
#include <Eigen/Core>
#include <iostream>

using namespace AXIOM;

AXIOM_EXPORT AxiomExecutionHandle AxiomExecution_Create() {
    try {
        return static_cast<void*>(new ExecutionOrchestrator());
    } catch (...) {
        return nullptr;
    }
}

AXIOM_EXPORT void AxiomExecution_Destroy(AxiomExecutionHandle handle) {
    if (handle) {
        delete static_cast<ExecutionOrchestrator*>(handle);
    }
}

AXIOM_EXPORT int AxiomExecution_DispatchMatrix(AxiomExecutionHandle handle, 
                                              const char* operation_name, 
                                              const double* raw_data, 
                                              size_t rows, 
                                              size_t cols) {
                             
    if (!handle || !operation_name || !raw_data) return -1;

    if (rows == 0 || cols == 0 || rows > 100000 || cols > 100000) return -1;

    try {
        auto* orchestrator = static_cast<ExecutionOrchestrator*>(handle);
        std::string op_str(operation_name);

        Eigen::Map<const Eigen::MatrixXd> mapped_matrix(raw_data, static_cast<Eigen::Index>(rows), static_cast<Eigen::Index>(cols));

        EngineResult result = orchestrator->DispatchMatrixOperation(op_str, mapped_matrix);

        if (result.HasErrors()) {
            return -3;
        }

        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[AXIOM FFI FATAL] " << e.what() << '\n';
        return -2;
    } catch (...) {
        return -2;
    }
}
