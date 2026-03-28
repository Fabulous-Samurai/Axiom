/**
 * @file execution_orchestrator.cpp
 * @brief Implementation of the ExecutionOrchestrator (formerly SelectiveDispatcher).
 */

#include "execution_orchestrator.h"
#include "signal_exec_traits.h"
#include "dynamic_calc.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <chrono>
#include <cctype>
#include <initializer_list>
#include <string_view>

#ifdef ENABLE_EIGEN
#include "eigen_engine.h"
#endif

#ifdef ENABLE_NANOBIND
#include "nanobind_interface.h"
#endif

namespace AXIOM {

namespace {

std::string ToLower(std::string text) {
    std::ranges::transform(text, text.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

bool MatchesOperation(std::string_view operation,
                      std::initializer_list<const char*> aliases) {
    for (const char* alias : aliases) {
        if (operation == alias) {
            return true;
        }
    }
    return false;
}

std::string BuildOperationInput(std::string_view operation,
                                const std::vector<std::string>& args) {
    std::string full_op{operation};
    size_t capacity = operation.size();
    for (const auto& arg : args) {
        capacity += 1 + arg.size();
    }
    full_op.reserve(capacity);

    for (const auto& arg : args) {
        full_op += ' ';
        full_op += arg;
    }
    return full_op;
}

ComputeEngine ResolveEngine(const ComputeEngine preferred_default,
                            const std::map<ComputeEngine, bool>& availability,
                            const ComputeEngine preferred_engine) {
    ComputeEngine engine = (preferred_engine == ComputeEngine::Auto)
        ? preferred_default : preferred_engine;

    const bool engine_ok = availability.contains(engine) &&
                           availability.at(engine);
    if (!engine_ok || engine == ComputeEngine::Auto) {
        return ComputeEngine::Native;
    }
    return engine;
}

bool RequiresSquareMatrix(std::string_view operation)
{
    return SignalExec::IsSquareOnlyOperation(operation);
}

StatusCode MapDispatchException(const std::exception& exception) {
    const std::string message = ToLower(exception.what());

    if (message.contains("divide") && message.contains("zero")) {
        return StatusCode::DivideByZero;
    }
    if (message.contains("parse") ||
        message.contains("syntax") ||
        message.contains("token")) {
        return StatusCode::ParseError;
    }
    if (message.contains("negative") && message.contains("root")) {
        return StatusCode::NegativeRoot;
    }
    if (message.contains("domain")) {
        return StatusCode::DomainError;
    }
    if (message.contains("argument") ||
        message.contains("mismatch") ||
        message.contains("invalid")) {
        return StatusCode::ArgumentMismatch;
    }
    if (message.contains("overflow")) {
        return StatusCode::NumericOverflow;
    }
    if (message.contains("memory") ||
        message.contains("alloc") ||
        message.contains("bad_alloc")) {
        return StatusCode::MemoryExhausted;
    }

    return StatusCode::OperationNotFound;
}

} // namespace

struct ExecutionOrchestrator::Impl {
    ComputeEngine preferred_engine_{ComputeEngine::Auto};
    bool fallback_enabled_{true};
    double performance_threshold_ms_{100.0};
    bool learning_enabled_{true};
    
    DispatchMetrics last_metrics_{};
    
    struct EngineStatus { ComputeEngine engine; bool available; };
    AXIOM::FixedVector<EngineStatus, 8> engine_availability_;
    
    // Performance tracking could be added here using FixedVector if needed
    
#ifdef ENABLE_EIGEN
    std::unique_ptr<EigenEngine> eigen_engine_;
#endif

#ifdef ENABLE_NANOBIND
    std::unique_ptr<NanobindInterface> nanobind_interface_;
#endif

    Impl() {
        engine_availability_.push_back({ComputeEngine::Native, true});
    }

    bool IsAvailable(ComputeEngine engine) const {
        for (const auto& status : engine_availability_) {
            if (status.engine == engine) return status.available;
        }
        return false;
    }

    void SetAvailable(ComputeEngine engine, bool available) {
        for (auto& status : engine_availability_) {
            if (status.engine == engine) {
                status.available = available;
                return;
            }
        }
        engine_availability_.push_back({engine, available});
    }
};

ExecutionOrchestrator::ExecutionOrchestrator() : pimpl_(std::make_unique<Impl>()) {

#ifdef ENABLE_EIGEN
    try {
        pimpl_->eigen_engine_ = std::make_unique<EigenEngine>();
        pimpl_->SetAvailable(ComputeEngine::Eigen, true);
    } catch (...) {
        pimpl_->SetAvailable(ComputeEngine::Eigen, false);
    }
#endif

#ifdef ENABLE_NANOBIND
    try {
        pimpl_->nanobind_interface_ = std::make_unique<NanobindInterface>();
        pimpl_->SetAvailable(ComputeEngine::Python, true);
    } catch (...) {
        pimpl_->SetAvailable(ComputeEngine::Python, false);
    }
#endif
}

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

    EngineResult result;
    
    // BUILD OPERATION INPUT - ZERO ALLOCATION
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
    
    std::string_view full_op(buffer, offset);
    
    thread_local DynamicCalc native;
    result = native.Evaluate(full_op);

    auto end = std::chrono::high_resolution_clock::now();
    pimpl_->last_metrics_.execution_time_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    pimpl_->last_metrics_.selected_engine  = engine;
    pimpl_->last_metrics_.operation_name   = operation;
    pimpl_->last_metrics_.fallback_used    = (engine != preferred_engine &&
                                              preferred_engine != ComputeEngine::Auto);
    return result;
}

EngineResult ExecutionOrchestrator::DispatchMatrixOperation(std::string_view operation,
                                                         const Eigen::Ref<const Eigen::MatrixXd>& matrix_data) {
#ifdef ENABLE_EIGEN
    if (!pimpl_->IsAvailable(ComputeEngine::Eigen) || !pimpl_->eigen_engine_) [[unlikely]] {
        return CreateErrorResult(StatusCode::OperationNotFound);
    }

    const int rows = static_cast<int>(matrix_data.rows());
    const int cols = static_cast<int>(matrix_data.cols());
    if (RequiresSquareMatrix(operation) && rows != cols) [[unlikely]] {
        return CreateErrorResult(StatusCode::ArgumentMismatch);
    }

    AXIOM::Matrix mat;
    for (int r = 0; r < rows; ++r) {
        AXIOM::Vector row;
        for (int c = 0; c < cols; ++c) {
            row.push_back(matrix_data(r, c));
        }
        mat.push_back(row);
    }

    const auto to_axiom_matrix = [](const auto& eigen_matrix) {
        AXIOM::Matrix out;
        for (int r = 0; r < eigen_matrix.rows(); ++r) {
            AXIOM::Vector row;
            for (int c = 0; c < eigen_matrix.cols(); ++c) {
                row.push_back(eigen_matrix(r, c));
            }
            out.push_back(row);
        }
        return out;
    };

    if (MatchesOperation(operation, {"determinant", "det"})) {
        const auto eigen_mat = pimpl_->eigen_engine_->CreateMatrix(mat);
        return CreateSuccessResult(pimpl_->eigen_engine_->Determinant(eigen_mat));
    }

    if (MatchesOperation(operation, {"transpose", "trans"})) {
        const auto eigen_mat = pimpl_->eigen_engine_->CreateMatrix(mat);
        auto t = pimpl_->eigen_engine_->Transpose(eigen_mat);
        return CreateSuccessResult(to_axiom_matrix(t));
    }

    if (MatchesOperation(operation, {"inverse", "inv"})) {
        const auto eigen_mat = pimpl_->eigen_engine_->CreateMatrix(mat);
        auto inv = pimpl_->eigen_engine_->Inverse(eigen_mat);
        return CreateSuccessResult(to_axiom_matrix(inv));
    }

    if (MatchesOperation(operation, {"eigenvalues", "eigvals"})) {
        const auto eigen_mat = pimpl_->eigen_engine_->CreateMatrix(mat);
        const auto [eigenvalues, eigenvectors] = pimpl_->eigen_engine_->EigenDecomposition(eigen_mat);
        AXIOM::Vector out;
        for (int i = 0; i < eigenvalues.size(); ++i) {
            out.push_back(eigenvalues(i));
        }
        return CreateSuccessResult(std::move(out));
    }

    if (operation == "trace") {
        double trace = 0.0;
        for (int i = 0; i < rows; ++i) {
            trace += mat[static_cast<size_t>(i)][static_cast<size_t>(i)];
        }
        return CreateSuccessResult(trace);
    }
#endif
    return CreateErrorResult(StatusCode::OperationNotFound);
}

std::string_view ExecutionOrchestrator::GetPerformanceReport() const {
    static char report_buffer[1024];
    int len = std::snprintf(report_buffer, sizeof(report_buffer),
        "[AXIOM Dispatch Report]\n"
        "  Last operation : %.*s\n"
        "  Engine used    : %d\n"
        "  Exec time (ms) : %.4f\n"
        "  Fallback used  : %s\n",
        (int)pimpl_->last_metrics_.operation_name.size(), pimpl_->last_metrics_.operation_name.data(),
        static_cast<int>(pimpl_->last_metrics_.selected_engine),
        pimpl_->last_metrics_.execution_time_ms,
        pimpl_->last_metrics_.fallback_used ? "yes" : "no");
    
    return std::string_view(report_buffer, len > 0 ? len : 0);
}

} // namespace AXIOM
