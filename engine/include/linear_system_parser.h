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
#include <memory_resource> // PMR allocators

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
        ArenaVector<CommandEntry> command_registry_;

        void RegisterCommands();

        EngineResult HandleQR(std::string_view input) const noexcept;
        EngineResult HandleEigen(std::string_view input) const noexcept;
        EngineResult HandleCramer(std::string_view input) const noexcept;
        EngineResult HandleSolve(std::string_view input) const noexcept;
        EngineResult HandleDefaultSolve(std::string_view input) const noexcept;

        ArenaVector<double> solve_linear_system(const ArenaVector<ArenaVector<double>>& A, const ArenaVector<double>& b) const noexcept;
        bool ParseLinearSystem(std::string_view input, ArenaVector<ArenaVector<double>> &A, ArenaVector<double> &b) const noexcept;
        ArenaVector<ArenaVector<double>> ParseMatrixString(std::string_view input) const noexcept;

        ArenaVector<ArenaVector<double>> GetMinor(const ArenaVector<ArenaVector<double>> &A, int row, int col) const noexcept;
        double Determinant(const ArenaVector<ArenaVector<double>> &A) const noexcept;
        std::optional<ArenaVector<double>> CramersRule(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<double> &b) const noexcept;
        ArenaVector<ArenaVector<double>> Transpose(const ArenaVector<ArenaVector<double>> &A) const noexcept;
        double DotProduct(const ArenaVector<double> &v1, const ArenaVector<double> &v2) const noexcept;
        double VectorNorm(const ArenaVector<double> &v) const noexcept;
        ArenaVector<double> VectorScale(const ArenaVector<double> &v, double scalar) const noexcept;
        ArenaVector<double> VectorSub(const ArenaVector<double> &v1, const ArenaVector<double> &v2) const noexcept;
        std::pair<ArenaVector<ArenaVector<double>>, ArenaVector<ArenaVector<double>>> GramSchmidt(const ArenaVector<ArenaVector<double>> &A) const noexcept;
        ArenaVector<ArenaVector<double>> MultiplyMatrices(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<ArenaVector<double>> &B) const noexcept;
        ArenaVector<ArenaVector<double>> CreateIdentityMatrix(int n) const noexcept;
        ArenaVector<double> GetDiagonal(const ArenaVector<ArenaVector<double>> &A) const noexcept;
        std::pair<ArenaVector<double>, ArenaVector<ArenaVector<double>>> ComputeEigenvalues(const ArenaVector<ArenaVector<double>> &A, int max_iterations = 100) const noexcept;
    };

    bool isCloseToZero(double value, double epsilon = 1e-9) noexcept;
    bool isValidNumber(std::string_view str) noexcept;

} // namespace AXIOM




