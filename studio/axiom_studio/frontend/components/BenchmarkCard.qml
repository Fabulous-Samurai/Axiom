import QtQuick
import "../theme"

GlassPanel {
    property string checkName: ""
    property real budget: 0
    property real measured: 0
    property string unit: ""
    property bool passed: true

    width: 300; height: 100

    Column {
        anchors.fill: parent; anchors.margins: Spacing.medium
        spacing: Spacing.small

        Row {
            width: parent.width
            Text {
                text: checkName
                font.pixelSize: Typography.bodyLarge
                font.weight: Typography.bold
                color: Colors.textPrimary
            }
            Item { width: parent.width - parent.children[0].width - parent.children[2].width }
            Text {
                text: passed ? "PASS" : "FAIL"
                color: passed ? Colors.success : Colors.error
                font.pixelSize: Typography.small
                font.weight: Typography.bold
            }
        }

        Row {
            spacing: Spacing.large
            Column {
                Text { text: "Measured"; color: Colors.textSecondary; font.pixelSize: Typography.small }
                Text {
                    text: measured.toFixed(2) + " " + unit
                    color: passed ? Colors.textPrimary : Colors.error
                    font.pixelSize: Typography.h3
                    font.family: Typography.code
                }
            }
            Column {
                Text { text: "Budget"; color: Colors.textSecondary; font.pixelSize: Typography.small }
                Text {
                    text: budget.toFixed(2) + " " + unit
                    color: Colors.textTertiary
                    font.pixelSize: Typography.h3
                    font.family: Typography.code
                }
            }
        }
    }
}