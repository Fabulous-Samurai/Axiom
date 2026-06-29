// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "ast_analyzer.h"
#include <variant>

namespace AXIOM {

void ASTAnalyzer::Analyze(NodePtr root) {
    analysis_cache_.clear();
    if (root) AnalyzeRecursive(root);
}

NodeAnalysis ASTAnalyzer::GetAnalysis(NodePtr node) const {
    auto it = analysis_cache_.find(node);
    if (it != analysis_cache_.end()) return it->second;
    return {};
}

void ASTAnalyzer::AnalyzeRecursive(NodePtr node) {
    if (!node) return;

    NodeAnalysis analysis;

    std::visit([&](auto&& n) {
        using T = std::decay_t<decltype(n)>;
        if constexpr (std::is_same_v<T, NumberNode>) {
            analysis.is_constant = true;
            analysis.complexity = OperationComplexity::Simple;
        } else if constexpr (std::is_same_v<T, VariableNode>) {
            analysis.is_constant = false;
            analysis.complexity = OperationComplexity::Simple;
        } else if constexpr (std::is_same_v<T, MatrixNode>) {
            analysis.is_constant = true;
            analysis.is_matrix = true;
            analysis.rows = n.matrix.size();
            analysis.cols = n.matrix.empty() ? 0 : n.matrix[0].size();
            analysis.complexity = OperationComplexity::Medium;
        } else if constexpr (std::is_same_v<T, BinaryOpNode>) {
            AnalyzeRecursive(n.left);
            AnalyzeRecursive(n.right);
            auto l = GetAnalysis(n.left);
            auto r = GetAnalysis(n.right);
            analysis.is_constant = l.is_constant && r.is_constant;
            analysis.complexity = OperationComplexity::Medium;
        } else if constexpr (std::is_same_v<T, UnaryOpNode>) {
            AnalyzeRecursive(n.operand);
            auto sub = GetAnalysis(n.operand);
            analysis.is_constant = sub.is_constant;
            analysis.complexity = OperationComplexity::Simple;
        } else if constexpr (std::is_same_v<T, MultiArgFunctionNode>) {
            bool all_const = true;
            for (auto arg : n.args) {
                AnalyzeRecursive(arg);
                if (!GetAnalysis(arg).is_constant) all_const = false;
            }
            analysis.is_constant = all_const;
            analysis.complexity = OperationComplexity::Complex;
        } else if constexpr (std::is_same_v<T, MatrixBinaryOpNode>) {
            AnalyzeRecursive(n.left);
            AnalyzeRecursive(n.right);
            auto l = GetAnalysis(n.left);
            auto r = GetAnalysis(n.right);
            analysis.is_constant = l.is_constant && r.is_constant;
            analysis.is_matrix = true;
            analysis.complexity = OperationComplexity::VeryComplex;
        }
    }, *node);

    analysis_cache_[node] = analysis;
}

} // namespace AXIOM
