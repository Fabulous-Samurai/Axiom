import QtQuick
import Qt5Compat.GraphicalEffects
import "../theme"

GlassPanel {
    id: root
    width: 300; height: 60
    property string message: ""
    
    x: parent.width
    opacity: 0
    visible: opacity > 0

    function show(msg) {
        message = msg;
        slideIn.start();
        hideTimer.restart();
    }

    Timer {
        id: hideTimer
        interval: 3000
        onTriggered: slideOut.start()
    }

    ParallelAnimation {
        id: slideIn
        NumberAnimation { target: root; property: "x"; to: parent.width - width - Spacing.large; duration: 200; easing.type: Easing.OutCubic }
        NumberAnimation { target: root; property: "opacity"; to: 1.0; duration: 200 }
    }

    ParallelAnimation {
        id: slideOut
        NumberAnimation { target: root; property: "x"; to: parent.width; duration: 200; easing.type: Easing.InCubic }
        NumberAnimation { target: root; property: "opacity"; to: 0.0; duration: 200 }
    }

    Row {
        anchors.fill: parent; anchors.margins: Spacing.medium
        spacing: Spacing.small
        
        Text {
            text: "ℹ️"
            font.pixelSize: Typography.h4
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: root.message
            color: Colors.textPrimary
            font.family: Typography.body
            font.pixelSize: Typography.bodyDefault
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 40
            wrapMode: Text.Wrap
        }
    }
}