import QtQuick 2.15

Rectangle {
    id: root
    property string variant: "rect"
    property bool animated: true

    color: "#334155"
    radius: variant === "circle" ? width / 2 : 4

    SequentialAnimation on opacity {
        running: root.animated
        loops: Animation.Infinite
        NumberAnimation { from: 0.3; to: 0.6; duration: 1000; easing.type: Easing.InOutQuad }
        NumberAnimation { from: 0.6; to: 0.3; duration: 1000; easing.type: Easing.InOutQuad }
    }
}
