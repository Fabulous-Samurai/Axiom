// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#include "axui/jit_execution_manager.h"
#include <unordered_map>

namespace axui {

JitExecutionManager::JitExecutionManager(QObject* parent) : QObject(parent) {}

void JitExecutionManager::compileExpression(const QString& expr) {
    m_lastExpression = expr;
    
    // CONVERT QSTRING TO BUFFER - ZERO HEAP IF POSSIBLE (USING LOCAL ARRAY)
    QByteArray utf8 = expr.toUtf8();
    std::string_view sv(utf8.constData(), static_cast<size_t>(utf8.size()));
    
    auto node = m_parser.ParseExpression(sv);
    if (!node) {
        Q_EMIT compilationError("Parsing failed: Invalid expression structure.");
        return;
    }

    // Use zero-allocation SymbolTable
    AXIOM::SymbolTable var_map;
    node->CollectVariables(var_map);
    
    auto fn = m_jit.Compile(node, var_map);
    if (!fn) {
        Q_EMIT compilationError("JIT Compilation failed: Check for illegal operations.");
        return;
    }

    Q_EMIT disassemblyChanged();
}

} // namespace axui
