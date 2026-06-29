import QtQuick
import QtQuick.Layouts
import "../theme"

Item {
    id: root
    anchors.fill: parent
    property string title: "Coming Soon"

    Column {
        anchors.centerIn: parent
        spacing: Spacing.large
        Text {
            text: "🚧"
            font.pixelSize: 64
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: root.title
            font.pixelSize: Typography.h2
            font.family: Typography.heading
            color: Colors.textPrimary
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: "This feature is currently under development."
            font.pixelSize: Typography.bodyLarge
            color: Colors.textSecondary
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}