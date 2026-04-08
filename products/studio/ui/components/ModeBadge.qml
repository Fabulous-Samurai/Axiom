import QtQuick
import "../theme"

Rectangle {
    property string mode: "algebraic"

    width: 80; height: 24
    radius: Spacing.radiusSmall
    color: "transparent"

    function getModeColor() {
        if (mode === "algebraic") return Colors.modeAlgebraic
        if (mode === "linear") return Colors.modeLinear
        if (mode === "statistics") return Colors.modeStatistics
        if (mode === "symbolic") return Colors.modeSymbolic
        if (mode === "units") return Colors.modeUnits
        if (mode === "plot") return Colors.modePlot
        return Colors.info
    }

    border.color: getModeColor()
    border.width: 1

    Text {
        anchors.centerIn: parent
        text: mode.toUpperCase()
        color: parent.border.color
        font.family: Typography.body
        font.pixelSize: Typography.xsmall
        font.weight: Typography.bold
    }
}