import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: root
    property string variant: "primary"

    contentItem: Text {
        text: root.text
        font: root.font
        color: root.down ? "#cbd5e1" : "#f8fafc"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        opacity: enabled ? 1 : 0.3
        color: root.down ? "#1e40af" : (root.hovered ? "#2563eb" : "#3b82f6")
        radius: 6
        border.color: "#2563eb"
        border.width: 1
    }
}
