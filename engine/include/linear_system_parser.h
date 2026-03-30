// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once
#include "IParser.h"
#include "dynamic_calc_types.h"
#include "arena_allocator.h"
#include "fixed_vector.h"
#include <string>
#include <functional>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h> // SIMD support
#endif
#include <unordered_map>
#include <memory_resource>

#include "axiom_export.h"

namespace AXIOM
{
    class AXIOM_EXPORT LinearSystemParser : public IParser
    {
    public:
        LinearSystemParser();
        EngineResult ParseAndExecute(std::string_view input) noexcept override;

    private:
        struct CommandEntry
        {
            std::string command;
            std::function<EngineResult(std::string_view)> handler;
            std::string description;
        };
        
        // Use engine standard Vector/Matrix (256 capacity)
        FixedVector<CommandEntry, 32> command_registry_;

        void RegisterCommands();

        EngineResult HandleQR(std::string_view input) const noexcept;
        EngineResult HandleEigen(std::string_view input) const noexcept;
        EngineResult HandleCramer(std::string_view input) const noexcept;
        EngineResult HandleSolve(std::string_view input) const noexcept;
        EngineResult HandleDefaultSolve(std::string_view input) const noexcept;

        Vector solve_linear_system(const Matrix& A, const Vector& b) const noexcept;
        bool ParseLinearSystem(std::string_view input, Matrix &A, Vector &b) const noexcept;
        Matrix ParseMatrixString(std::string_view input) const noexcept;

        Matrix GetMinor(const Matrix &A, int row, int col) const noexcept;
        double Determinant(const Matrix &A) const noexcept;
        std::optional<Vector> CramersRule(const Matrix &A, const Vector &b) const noexcept;
        Matrix Transpose(const Matrix &A) const noexcept;
        double DotProduct(const Vector &v1, const Vector &v2) const noexcept;
        double VectorNorm(const Vector &v) const noexcept;
        Vector VectorScale(const Vector &v, double scalar) const noexcept;
        Vector VectorSub(const Vector &v1, const Vector &v2) const noexcept;
        std::pair<Matrix, Matrix> GramSchmidt(const Matrix &A) const noexcept;
        Matrix MultiplyMatrices(const Matrix &A, const Matrix &B) const noexcept;
        Matrix CreateIdentityMatrix(int n) const noexcept;
        Vector GetDiagonal(const Matrix &A) const noexcept;
        std::pair<Vector, Matrix> ComputeEigenvalues(const Matrix &A, int max_iterations = 100) const noexcept;
    };

    bool isCloseToZero(double value, double epsilon = 1e-9) noexcept;
    bool isValidNumber(std::string_view str) noexcept;

} // namespace AXIOM
