import QtQuick 2.15
import QtQuick.Layouts 1.15

/*
 * TelemetryGauge — Inline horizontal gauge for system metrics
 *
 * CPU ████████░░░░ 78%
 */

Item {
    id: root

    property string label: "CPU"
    property real value: 0
    property real maxValue: 100
    property string unit: "%"
    property string subtitle: ""
    property color color: "#3B82F6"
    property bool enabled: true

    implicitHeight: 48

    readonly property real ratio: enabled ? Math.min(1, value / maxValue) : 0

    readonly property color barColor: {
        if (!enabled) return "#334155"
        if (ratio > 0.9) return "#EF4444"
        if (ratio > 0.75) return "#EAB308"
        return color
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        // Label + Value
        RowLayout {
            spacing: 8

            Text {
                text: label
                font.pixelSize: 11
                font.weight: Font.Medium
                color: enabled ? "#94A3B8" : "#475569"
            }

            Item { Layout.fillWidth: true }

            Text {
                text: enabled ? value.toFixed(value >= 100 ? 0 : 1) + unit : "N/A"
                font.pixelSize: 13
                font.weight: Font.Bold
                font.family: "JetBrains Mono"
                color: enabled ? barColor : "#475569"
            }
        }

        // Bar
        Rectangle {
            Layout.fillWidth: true
            height: 6
            radius: 3
            color: "#1E293B"

            Rectangle {
                width: parent.width * ratio
                height: parent.height
                radius: 3
                color: barColor

                Behavior on width {
                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                }

                Behavior on color {
                    ColorAnimation { duration: 300 }
                }
            }
        }

        // Subtitle
        Text {
            visible: subtitle !== ""
            text: subtitle
            font.pixelSize: 9
            color: "#475569"
            font.family: "JetBrains Mono"
        }
    }
}
