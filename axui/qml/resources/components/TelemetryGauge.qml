import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════
    property string label: "METRIC"
    property real value: 0
    property string unit: "%"
    property real warningThreshold: 70
    property real criticalThreshold: 90

    // Internal Smoothing
    property real smoothValue: 0
    property real emaAlpha: 0.15
    onValueChanged: {
        smoothValue = (value * emaAlpha) + (smoothValue * (1.0 - emaAlpha))
    }

    width: 120
    height: 60

    // Severity Logic
    readonly property color severityColor: {
        if (smoothValue > criticalThreshold) return "#EF4444"
        if (smoothValue > warningThreshold) return "#F59E0B"
        return "#3B82F6" // Standard Blue
    }

    // ═══════════════════════════════════════════════════════════════
    // UI COMPONENTS
    // ═══════════════════════════════════════════════════════════════

    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        Text {
            text: label.toUpperCase()
            font.pixelSize: 10
            font.weight: Font.Bold
            font.family: "JetBrains Mono"
            color: "#94A3B8"
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: smoothValue.toFixed(1) + unit
            font.pixelSize: 20
            font.weight: Font.Bold
            font.family: "JetBrains Mono"
            color: severityColor
            Layout.alignment: Qt.AlignHCenter

            Behavior on color { ColorAnimation { duration: 500 } }
        }

        // Mini Progress line
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: "#334155"
            radius: 1

            Rectangle {
                width: Math.min(parent.width, (smoothValue / 100) * parent.width)
                height: parent.height
                radius: 1
                color: severityColor
                Behavior on width { NumberAnimation { duration: 300 } }
            }
        }
    }
}
