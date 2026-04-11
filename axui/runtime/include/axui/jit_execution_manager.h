// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
#pragma once

#include <QObject>
#include <QString>

#ifdef emit
#undef emit
#endif

#include "zenith_jit.h"
#include "algebraic_parser.h"

namespace axui {

/**
 * @brief Manages JIT compilation and execution for the UI layer.
 * Renamed from ZenithManager for architectural clarity.
 */
class JitExecutionManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString disassembly READ disassembly NOTIFY disassemblyChanged)
    Q_PROPERTY(QString lastExpression READ lastExpression NOTIFY disassemblyChanged)

public:
    explicit JitExecutionManager(QObject* parent = nullptr);

    QString disassembly() const {
        const auto& disasm = m_jit.last_disassembly();
        return QString::fromUtf8(disasm.data(), static_cast<int>(disasm.size()));
    }
    QString lastExpression() const { return m_lastExpression; }

    Q_INVOKABLE void compileExpression(const QString& expr);

signals:
    void disassemblyChanged();
    void compilationError(const QString& error);

private:
    AXIOM::ZenithJIT m_jit;
    AXIOM::AlgebraicParser m_parser;
    QString m_lastExpression;
};

} // namespace axui
