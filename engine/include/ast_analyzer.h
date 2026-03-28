// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "ast_nodes.h"
#include <unordered_map>

namespace AXIOM {

/**
 * @brief Analysis result for a single AST node.
 */
struct NodeAnalysis {
    bool is_matrix = false;
    int rows = 0;
    int cols = 0;
};

/**
 * @brief AST Matrix Analyzer
 * 
 * Pre-identifies matrix vs scalar operations in the AST to assist ZenithJIT
 * in register allocation and SIMD dispatch.
 */
class ASTAnalyzer {
public:
    ASTAnalyzer() = default;

    /**
     * @brief Analyze the entire AST starting from root.
     */
    void Analyze(const ExprNode* root);

    /**
     * @brief Get analysis for a specific node.
     */
    NodeAnalysis GetAnalysis(const ExprNode* node) const;

    /**
     * @brief Check if a node produces a matrix.
     */
    bool IsMatrix(const ExprNode* node) const {
        return GetAnalysis(node).is_matrix;
    }

private:
    void AnalyzeRecursive(const ExprNode* node);

    std::unordered_map<const ExprNode*, NodeAnalysis> analysis_cache_;
};

} // namespace AXIOM
