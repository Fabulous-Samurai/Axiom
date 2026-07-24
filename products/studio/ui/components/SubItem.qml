import QtQuick
import "../theme"

Rectangle {
    id: root
    property color badge: Colors.primary
    property string text: ""
    signal clicked()

    width: parent ? parent.width : 200; height: 32
    color: hoverArea.containsMouse ? Colors.hover : "transparent"

    Row {
        anchors.fill: parent
        anchors.leftMargin: Spacing.xlarge
        spacing: Spacing.medium

        Rectangle {
            width: 8; height: 8; radius: 4
            color: root.badge
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.text
            color: Colors.textSecondary
            font.family: Typography.body
            font.pixelSize: Typography.small
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
