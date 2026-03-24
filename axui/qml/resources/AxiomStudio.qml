import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Window {
    id: studioRoot
    visible: true
    width: 1280
    height: 720
    color: "#050505"
    title: "AXIOM Studio | Integrated Development Environment"

    // Background Depth
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0F172A" }
            GradientStop { position: 1.0; color: "#020617" }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20

        Text {
            text: "AXIOM STUDIO"
            font.pixelSize: 48
            font.weight: Font.Black
            font.letterSpacing: 10
            color: "#3B82F6"
            opacity: 0.8
        }

        Text {
            text: "ARCHITECTURAL PROTOTYPE READY"
            font.pixelSize: 14
            font.letterSpacing: 4
            color: "#64748B"
            Layout.alignment: Qt.AlignCenter
        }

        Rectangle {
            width: 300
            height: 2
            color: "#1E293B"
            Layout.alignment: Qt.AlignCenter
        }

        Text {
            text: "TARGET: axiom_studio\nWORKSPACE: Ready\nIPC: Connected"
            font.pixelSize: 11
            font.family: "JetBrains Mono"
            color: "#475569"
            horizontalAlignment: Text.AlignHCenter
            lineHeight: 1.5
        }
    }
}
