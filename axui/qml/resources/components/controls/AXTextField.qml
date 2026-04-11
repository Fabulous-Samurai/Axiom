import QtQuick 2.15
import QtQuick.Controls 2.15

TextField {
    id: root

    placeholderTextColor: "#94a3b8"
    color: "#f8fafc"
    selectionColor: "#3b82f6"
    selectedTextColor: "#ffffff"

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        color: "#1e293b"
        radius: 6
        border.color: root.focus ? "#3b82f6" : "#334155"
        border.width: root.focus ? 2 : 1
    }
}
