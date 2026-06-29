// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include "ast_nodes.h"
#include <unordered_map>

namespace AXIOM {

struct NodeAnalysis {
    bool is_constant = false;
    bool is_matrix = false;
    size_t rows = 0;
    size_t cols = 0;
    OperationComplexity complexity = OperationComplexity::Simple;
};

/**
 * @brief Analyzes AST nodes for optimization and dispatch decisions.
 * Updated for Operation VARIANT SHIFT (no virtual calls).
 */
class ASTAnalyzer {
public:
    void Analyze(NodePtr root);

    // Returns cached analysis for a node
    NodeAnalysis GetAnalysis(NodePtr node) const;

    bool IsMatrix(NodePtr node) const {
        return GetAnalysis(node).is_matrix;
    }

private:
    void AnalyzeRecursive(NodePtr node);

    std::unordered_map<NodePtr, NodeAnalysis> analysis_cache_;
};

} // namespace AXIOM
