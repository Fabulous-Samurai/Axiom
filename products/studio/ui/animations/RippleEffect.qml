import QtQuick
import "../theme"

Rectangle {
    id: rippleCircle
    width: 0; height: 0
    radius: width / 2
    color: Qt.rgba(1.0, 1.0, 1.0, 0.3)
    anchors.centerIn: parent

    function start() {
        rippleAnimation.restart()
    }

    ParallelAnimation {
        id: rippleAnimation
        PropertyAnimation {
            target: rippleCircle; property: "width"
            from: 0; to: parent.width * 2
            duration: 400; easing.type: Easing.OutCubic
        }
        PropertyAnimation {
            target: rippleCircle; property: "opacity"
            from: 1; to: 0
            duration: 400
        }
    }
}
