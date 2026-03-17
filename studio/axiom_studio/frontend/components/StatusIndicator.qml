import QtQuick
import "../theme"

Row {
    property string status: "running"
    property string label: ""
    
    spacing: Spacing.small

    Rectangle {
        width: 8; height: 8; radius: 4
        color: status === "running" ? Colors.success : (status === "error" ? Colors.error : Colors.warning)
        anchors.verticalCenter: parent.verticalCenter
    }
    
    Text {
        text: label
        color: Colors.textSecondary
        font.family: Typography.body
        font.pixelSize: Typography.small
        anchors.verticalCenter: parent.verticalCenter
    }
}