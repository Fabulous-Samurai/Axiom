import QtQuick
import "../theme"

Rectangle {
    property string source: ""

    color: Colors.canvas
    border.color: Colors.border
    border.width: 1
    radius: Spacing.radiusMedium
    clip: true

    Image {
        id: img
        anchors.fill: parent
        anchors.margins: 1
        source: parent.source
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: false
    }

    Text {
        visible: img.status !== Image.Ready
        anchors.centerIn: parent
        text: "No Output"
        color: Colors.textTertiary
        font.pixelSize: Typography.bodyLarge
    }
}