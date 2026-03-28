// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "ast_analyzer.h"

namespace AXIOM {

void ASTAnalyzer::Analyze(const ExprNode* root) {
    analysis_cache_.clear();
    if (!root) return;
    AnalyzeRecursive(root);
}

NodeAnalysis ASTAnalyzer::GetAnalysis(const ExprNode* node) const {
    auto it = analysis_cache_.find(node);
    if (it != analysis_cache_.end()) {
        return it->second;
    }
    return {}; // Default: Scalar
}

void ASTAnalyzer::AnalyzeRecursive(const ExprNode* node) {
    if (!node) return;

    NodeAnalysis analysis;
    NodeType type = node->GetType();

    switch (type) {
        case NodeType::Number:
        case NodeType::Variable:
            analysis.is_matrix = false;
            break;

        case NodeType::Matrix: {
            analysis.is_matrix = true;
            auto dims = node->GetMatrixDims();
            analysis.rows = dims.first;
            analysis.cols = dims.second;
            break;
        }

        case NodeType::MatrixBinaryOp: {
            analysis.is_matrix = true;
            const auto* mnode = static_cast<const MatrixBinaryOpNode*>(node);
            AnalyzeRecursive(mnode->left);
            AnalyzeRecursive(mnode->right);
            auto dims = mnode->left->GetMatrixDims();
            analysis.rows = dims.first;
            analysis.cols = dims.second;
            break;
        }

        case NodeType::BinaryOp: {
            const auto* bnode = static_cast<const BinaryOpNode*>(node);
            AnalyzeRecursive(bnode->left);
            AnalyzeRecursive(bnode->right);
            
            auto left_an = GetAnalysis(bnode->left);
            auto right_an = GetAnalysis(bnode->right);
            
            if (left_an.is_matrix || right_an.is_matrix) {
                analysis.is_matrix = true;
                analysis.rows = left_an.is_matrix ? left_an.rows : right_an.rows;
                analysis.cols = left_an.is_matrix ? left_an.cols : right_an.cols;
            } else {
                analysis.is_matrix = false;
            }
            break;
        }

        case NodeType::UnaryOp: {
            const auto* unode = static_cast<const UnaryOpNode*>(node);
            AnalyzeRecursive(unode->operand);
            auto op_an = GetAnalysis(unode->operand);
            
            // Check if function transforms scalar to matrix or vice versa
            // For now, assume unary ops preserve matrix-ness (e.g., sin(Matrix))
            analysis.is_matrix = op_an.is_matrix;
            analysis.rows = op_an.rows;
            analysis.cols = op_an.cols;
            break;
        }

        case NodeType::MultiArgFunction: {
            const auto* fnode = static_cast<const MultiArgFunctionNode*>(node);
            bool any_matrix = false;
            for (auto arg : fnode->args) {
                AnalyzeRecursive(arg);
                if (GetAnalysis(arg).is_matrix) {
                    any_matrix = true;
                    analysis.rows = GetAnalysis(arg).rows;
                    analysis.cols = GetAnalysis(arg).cols;
                }
            }
            analysis.is_matrix = any_matrix;
            break;
        }
    }

    analysis_cache_[node] = analysis;
}

} // namespace AXIOM
