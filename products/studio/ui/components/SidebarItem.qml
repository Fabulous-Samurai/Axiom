import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root
    property string iconSource: ""
    property string text: ""
    property bool active: false
    property bool isExpanded: false
    signal clicked()

    Layout.fillWidth: true
    Layout.preferredHeight: 56
    color: "transparent"

    Rectangle {
        anchors.fill: parent
        anchors.margins: { top: 4; bottom: 4; left: 8; right: 8 }
        radius: 8
        color: root.active ? Qt.rgba(125/255, 211/255, 252/255, 0.15) :
               (mArea.containsMouse ? Qt.rgba(255/255, 255/255, 255/255, 0.05) : "transparent")

        Behavior on color { ColorAnimation { duration: 150 } }

        // Dirac Accent Line
        Rectangle {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            width: 4
            height: root.active ? parent.height * 0.5 : 0
            color: "#7DD3FC"
            radius: 2
            Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: root.isExpanded ? 16 : 0
            spacing: 16

            Item {
                Layout.preferredWidth: root.isExpanded ? 24 : parent.width
                Layout.fillHeight: true

                Image {
                    anchors.centerIn: parent
                    width: 24; height: 24
                    source: root.iconSource != "" ? "file:///" + root.iconSource : ""
                    fillMode: Image.PreserveAspectFit
                    opacity: root.active ? 1.0 : 0.6
                    visible: root.iconSource != ""
                    sourceSize.width: 24
                    sourceSize.height: 24
                    antialiasing: true
                    Behavior on opacity { NumberAnimation { duration: 150 } }
                }
            }

            Text {
                Layout.fillWidth: true
                text: root.text
                color: root.active ? "#7DD3FC" : "#94A3B8"
                font.pixelSize: 12
                font.bold: true
                font.letterSpacing: 1.5
                visible: root.isExpanded
                opacity: root.isExpanded ? 1.0 : 0.0
                Behavior on opacity { NumberAnimation { duration: 200 } }
                Behavior on color { ColorAnimation { duration: 150 } }
            }
        }
    }

    MouseArea {
        id: mArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
