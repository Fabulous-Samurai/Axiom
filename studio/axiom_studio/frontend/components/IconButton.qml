import QtQuick
import "../theme"

Rectangle {
    property string iconText: ""
    signal clicked()

    width: 32; height: 32
    radius: Spacing.radiusMedium
    color: hoverArea.containsMouse ? Colors.hover : "transparent"

    Text {
        anchors.centerIn: parent
        text: iconText
        color: Colors.textPrimary
        font.pixelSize: Typography.h4
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }
}