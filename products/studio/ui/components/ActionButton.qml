import QtQuick
import "../theme"

Rectangle {
    id: root
    property string text: ""
    property string variant: "primary" // primary, secondary, ghost, destructive
    signal clicked()

    width: Math.max(100, label.width + Spacing.mediumLarge * 2)
    height: 36
    radius: Spacing.radiusMedium

    color: {
        if (variant === "primary") return Colors.primary;
        if (variant === "secondary") return Colors.secondary;
        if (variant === "destructive") return Colors.error;
        return "transparent";
    }

    border.color: variant === "ghost" ? Colors.border : "transparent"
    border.width: variant === "ghost" ? 1 : 0

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: variant === "ghost" ? Colors.textPrimary : Colors.surface
        font.family: Typography.body
        font.pixelSize: Typography.bodyLarge
        font.weight: Typography.medium
    }

    Rectangle {
        id: rippleCircle
        width: 0; height: 0
        radius: width / 2
        color: Qt.rgba(1.0, 1.0, 1.0, 0.3)
        anchors.centerIn: parent

        ParallelAnimation {
            id: rippleAnimation
            PropertyAnimation {
                target: rippleCircle; property: "width"
                from: 0; to: root.width * 2
                duration: 400; easing.type: Easing.OutCubic
            }
            PropertyAnimation {
                target: rippleCircle; property: "opacity"
                from: 1; to: 0
                duration: 400
            }
        }
    }

    scale: mouseArea.pressed ? 0.95 : 1.0
    Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: Qt.rgba(0, 0, 0, mouseArea.containsMouse ? 0.1 : 0.0)
        Behavior on color { ColorAnimation { duration: 150 } }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            rippleAnimation.restart()
            root.clicked()
        }
    }
}
