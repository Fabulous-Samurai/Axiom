import QtQuick
import QtQuick.Layouts

Rectangle {
    id: badgeRoot
    property string label: ""
    property string badgeValue: ""
    property color accentColor: "#7DD3FC" // İsim değişti

    width: 140
    height: 32

    color: "#1e293b"
    radius: 4

    RowLayout {
        anchors.centerIn: parent
        spacing: 8

        Text {
            text: badgeRoot.label
            color: "#64748B"
            font.pixelSize: 10
            font.bold: true
        }

        Text {
            text: badgeRoot.badgeValue
            color: badgeRoot.accentColor

            font.pixelSize: 11
            font.bold: true
        }
    }
}
