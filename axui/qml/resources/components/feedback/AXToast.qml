import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    property string message: ""
    property string type: "info"
    property int duration: 3000
    
    implicitWidth: messageText.implicitWidth + 48
    implicitHeight: 48
    radius: 8
    color: "#1e293b"
    border.color: "#334155"
    border.width: 1
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        
        Text {
            id: messageText
            text: root.message
            color: "#f8fafc"
            font.pixelSize: 14
        }
    }
}
