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
        EngineResult ParseAndExecute(const std::string &input) override;

    private:
        struct CommandEntry
        {
            std::string command;
            std::function<EngineResult(const std::string&)> handler;
            std::string description;
        };
        ArenaVector<CommandEntry> command_registry_;

        void RegisterCommands();

        EngineResult HandleQR(const std::string &input) const;
        EngineResult HandleEigen(const std::string &input) const;
        EngineResult HandleCramer(const std::string &input) const;
        EngineResult HandleSolve(const std::string &input) const;
        EngineResult HandleDefaultSolve(const std::string &input) const;

        ArenaVector<double> solve_linear_system(const ArenaVector<ArenaVector<double>>& A, const ArenaVector<double>& b) const;
        bool ParseLinearSystem(const std::string &input, ArenaVector<ArenaVector<double>> &A, ArenaVector<double> &b) const;
        ArenaVector<ArenaVector<double>> ParseMatrixString(const std::string &input) const;

        ArenaVector<ArenaVector<double>> GetMinor(const ArenaVector<ArenaVector<double>> &A, int row, int col) const;
        double Determinant(const ArenaVector<ArenaVector<double>> &A) const;
        std::optional<ArenaVector<double>> CramersRule(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<double> &b) const;
        ArenaVector<ArenaVector<double>> Transpose(const ArenaVector<ArenaVector<double>> &A) const;
        double DotProduct(const ArenaVector<double> &v1, const ArenaVector<double> &v2) const;
        double VectorNorm(const ArenaVector<double> &v) const;
        ArenaVector<double> VectorScale(const ArenaVector<double> &v, double scalar) const;
        ArenaVector<double> VectorSub(const ArenaVector<double> &v1, const ArenaVector<double> &v2) const;
        std::pair<ArenaVector<ArenaVector<double>>, ArenaVector<ArenaVector<double>>> GramSchmidt(const ArenaVector<ArenaVector<double>> &A) const;
        ArenaVector<ArenaVector<double>> MultiplyMatrices(const ArenaVector<ArenaVector<double>> &A, const ArenaVector<ArenaVector<double>> &B) const;
        ArenaVector<ArenaVector<double>> CreateIdentityMatrix(int n) const;
        ArenaVector<double> GetDiagonal(const ArenaVector<ArenaVector<double>> &A) const;
        std::pair<ArenaVector<double>, ArenaVector<ArenaVector<double>>> ComputeEigenvalues(const ArenaVector<ArenaVector<double>> &A, int max_iterations = 100) const;
    };

    bool isCloseToZero(double value, double epsilon = 1e-9);
    bool isValidNumber(const std::string &str);

} // namespace AXIOM




