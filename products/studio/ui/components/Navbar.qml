import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects
import "../theme"

Rectangle {
    height: Spacing.navbarHeight
    color: Colors.blurredSurface(0.8)

    layer.enabled: true
    layer.effect: FastBlur { radius: 24 }

    RowLayout {
        anchors.fill: parent
        anchors.margins: Spacing.medium
        spacing: Spacing.medium

        Row {
            spacing: Spacing.small
            Layout.alignment: Qt.AlignVCenter
            Rectangle {
                width: 12; height: 12; radius: 6
                color: Colors.accent
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: "AXIOM Studio v1.0.0"
                color: Colors.textPrimary
                font.family: Typography.heading
                font.pixelSize: Typography.h4
                font.weight: Typography.semibold
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Item { Layout.fillWidth: true }

        StatusIndicator {
            status: engineBridge.engineStatus
            label: "Engine v3.1.1"
            Layout.alignment: Qt.AlignVCenter
        }

        Row {
            spacing: Spacing.small
            Layout.alignment: Qt.AlignVCenter
            IconButton { iconText: "🔔" }
            IconButton { iconText: "⚙️" }
            IconButton { iconText: "👤" }
        }
    }
}