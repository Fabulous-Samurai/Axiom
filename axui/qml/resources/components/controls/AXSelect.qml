import QtQuick 2.15
import QtQuick.Controls 2.15

ComboBox {
    id: root
    property bool searchable: false
    property var options: []
    
    model: options
    
    contentItem: Text {
        leftPadding: 12
        rightPadding: root.indicator.width + root.spacing
        text: root.displayText
        font: root.font
        color: "#f8fafc"
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        color: "#1e293b"
        border.color: root.focus ? "#3b82f6" : "#334155"
        border.width: root.visualFocus ? 2 : 1
        radius: 6
    }
}
