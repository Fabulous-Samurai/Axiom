import QtQuick

Transition {
    PropertyAnimation {
        property: "x"; from: width; to: 0
        duration: 300; easing.type: Easing.OutCubic
    }
    PropertyAnimation {
        property: "opacity"; from: 0; to: 1
        duration: 300
    }
}
