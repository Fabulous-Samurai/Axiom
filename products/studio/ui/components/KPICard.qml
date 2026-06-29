import QtQuick
import Qt5Compat.GraphicalEffects
import "../theme"

GlassPanel {
    id: root

    property string title: ""
    property real value: 0
    property string unit: ""
    property real trend: 0
    property color accentColor: Colors.primary

    width: 280; height: 140

    function formatNumber(num) {
        return num.toLocaleString(Qt.locale(), 'f', 0);
    }

    layer.enabled: true
    layer.effect: Glow {
        radius: hoverArea.containsMouse ? 12 : 0
        color: root.accentColor
        opacity: hoverArea.containsMouse ? 0.3 : 0
        Behavior on radius { NumberAnimation { duration: 200 } }
        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    Column {
        anchors.fill: parent; anchors.margins: Spacing.mediumLarge
        spacing: Spacing.small

        Text {
            text: root.title
            color: Colors.textSecondary
            font.family: Typography.body
            font.pixelSize: Typography.small
        }

        Text {
            text: formatNumber(root.value) + " " + root.unit
            color: Colors.textPrimary
            font.family: Typography.heading
            font.pixelSize: Typography.metric
            font.weight: Typography.bold

            Behavior on text {
                // Not perfectly supported for string but ok
            }
        }

        Row {
            spacing: Spacing.micro
            visible: root.trend !== 0

            Text {
                text: root.trend > 0 ? "↑" : "↓"
                color: root.trend > 0 ? Colors.success : Colors.error
                font.pixelSize: Typography.bodyDefault
            }
            Text {
                text: Math.abs(root.trend).toFixed(1) + "%"
                color: root.trend > 0 ? Colors.success : Colors.error
                font.pixelSize: Typography.small
                font.weight: Typography.medium
            }
        }
    }

    scale: hoverArea.containsMouse ? 1.02 : 1.0
    Behavior on scale {
        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
    }
}