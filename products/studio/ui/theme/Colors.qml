pragma Singleton
import QtQuick

QtObject {
    id: colors

    property string currentTheme: "shadow_soft"

    readonly property color surface:        "#1E293B"
    readonly property color canvas:         "#334155"
    readonly property color backdrop:       "#0F172A"
    readonly property color primary:        "#7DD3FC"
    readonly property color secondary:      "#C4B5FD"
    readonly property color accent:         "#67E8F9"
    readonly property color success:        "#6EE7B7"
    readonly property color warning:        "#FCD34D"
    readonly property color error:          "#FCA5A5"
    readonly property color info:           "#93C5FD"
    readonly property color textPrimary:    "#E2E8F0"
    readonly property color textSecondary:  "#94A3B8"
    readonly property color textTertiary:   "#64748B"
    readonly property color textInverse:    "#1E293B"
    readonly property color border:         "#475569"
    readonly property color borderFocus:    "#7DD3FC"
    readonly property color hover:          "#334155"
    readonly property color active:         "#155E75"
    readonly property color disabled:       "#334155"

    readonly property color modeAlgebraic:  "#8B5CF6"
    readonly property color modeLinear:     "#3B82F6"
    readonly property color modeStatistics: "#14B8A6"
    readonly property color modeSymbolic:   "#F59E0B"
    readonly property color modeUnits:      "#EC4899"
    readonly property color modePlot:       "#10B981"

    readonly property color perfFast:       "#10B981"
    readonly property color perfOk:         "#3B82F6"
    readonly property color perfSlow:       "#F59E0B"
    readonly property color perfCritical:   "#EF4444"

    readonly property color verified:       "#10B981"
    readonly property color unverified:     "#F59E0B"
    readonly property color violation:      "#EF4444"

    readonly property color gpuActive:      "#22D3EE"
    readonly property color gpuIdle:        "#64748B"
    readonly property color gpuTransfer:    "#A78BFA"

    readonly property color mantisExplored: "#3B82F6"
    readonly property color mantisPruned:   "#EF4444"
    readonly property color mantisSolution: "#10B981"

    function glass(opacity) {
        return Qt.rgba(1.0, 1.0, 1.0, opacity)
    }

    function glassBackground() {
        return Qt.rgba(1.0, 1.0, 1.0, 0.1)
    }

    function glassBorder() {
        return Qt.rgba(1.0, 1.0, 1.0, 0.2)
    }

    function blurredSurface(opacity) {
        return Qt.rgba(0.118, 0.161, 0.231, opacity)
    }
}
