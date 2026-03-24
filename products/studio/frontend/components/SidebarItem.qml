import QtQuick
import "../theme"

Rectangle {
    id: root
    property string icon: ""
    property string text: ""
    property string page: ""
    property bool active: false
    property bool expandable: false
    signal clicked()

    width: parent.width; height: 40
    color: hoverArea.containsMouse ? Colors.hover : "transparent"

    Row {
        anchors.fill: parent
        anchors.margins: Spacing.medium
        spacing: Spacing.medium

        Text {
            text: root.icon
            font.pixelSize: Typography.h4
            anchors.verticalCenter: parent.verticalCenter
            width: 24
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            text: root.text
            color: root.active ? Colors.primary : Colors.textPrimary
            font.family: Typography.body
            font.pixelSize: Typography.bodyDefault
            font.weight: root.active ? Typography.semibold : Typography.medium
            anchors.verticalCenter: parent.verticalCenter
            visible: parent.width > 100
        }
    }

    Rectangle {
        width: 3; height: parent.height
        color: Colors.primary
        visible: root.active
        anchors.left: parent.left
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}