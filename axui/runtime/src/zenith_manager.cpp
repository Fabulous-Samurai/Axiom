// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "axui/zenith_manager.h"
#include <unordered_map>

namespace axui {

ZenithManager::ZenithManager(QObject* parent) : QObject(parent) {}

void ZenithManager::compileExpression(const QString& expr) {
    m_lastExpression = expr;
    
    try {
        auto node = m_parser.ParseExpression(expr.toStdString());
        if (!node) {
            Q_EMIT compilationError("Parsing failed: Invalid expression structure.");
            return;
        }

        // Dynamically collect variables from the AST
        std::unordered_map<std::string, int> var_map;
        node->CollectVariables(var_map);
        
        auto fn = m_jit.Compile(node, var_map);
        if (!fn) {
            Q_EMIT compilationError("JIT Compilation failed: Check for illegal operations.");
            return;
        }

        Q_EMIT disassemblyChanged();
    } catch (const std::exception& e) {
        Q_EMIT compilationError(QString("Exception: ") + e.what());
    }
}

} // namespace axui
