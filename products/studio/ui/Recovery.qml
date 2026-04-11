import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
    visible: true
    width: 600
    height: 400
    title: "AXIOM Studio | RECOVERY MODE"
    color: "#1e1e1e"

    Column {
        anchors.centerIn: parent
        spacing: 20
        width: parent.width - 40

        Text {
            text: "⚠️ UI LOAD FAILURE"
            color: "#f87171"
            font.pixelSize: 24
            font.bold: true
        }

        Rectangle {
            width: parent.width
            height: 200
            color: "#000000"
            border.color: "#333333"

            Flickable {
                anchors.fill: parent
                contentHeight: logText.height
                clip: true

                Text {
                    id: logText
                    width: parent.width - 10
                    text: "Checking logs..."
                    color: "#4ade80"
                    font.family: "Consolas"
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }
        }

        Button {
            text: "Retry Reload"
            onClicked: { /* Reload logic can be added */ }
        }
    }
}
