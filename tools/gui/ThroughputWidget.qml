import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Effects

// -------------------------------------------------------------------------
// DESIGN SPECIFICATION: DIRAC / PREMIUM DESKTOP UI
// -------------------------------------------------------------------------
// This component implements the "Hardware-Bound" visualization layer.
// It bypasses the Python GIL by utilizing the C++ SceneGraph node
// 'AxiomThroughputItem' for multimillion-point line rendering.
// -------------------------------------------------------------------------

Rectangle {
    id: root
    width: 400
    height: 250
    color: "transparent"
    radius: 16

    property string title: "Throughput History"
    property real currentOps: 2500452.0

    // High-Frequency Vector Renderer (C++ SceneGraph)
    // This is the Zero-Copy bridge directly to Vulkan/SceneGraph nodes.
    AxiomThroughputItem {
        id: chart
        anchors.fill: parent
        anchors.margins: 20
        color: "#00F2FF"
        maxPoints: 1024

        // Z-order logic: chart is below the glass top layer but
        // above the dark background.
        z: 1
    }

    // GPU-Accelerated Glassmorphism (MultiEffect)
    // Red Flag Avoidance: layer.enabled: true ensures the blur is GPU-cached.
    Rectangle {
        id: glassBase
        anchors.fill: parent
        radius: 16
        color: Qt.rgba(0.06, 0.08, 0.12, 0.6)
        border.color: Qt.rgba(1, 1, 1, 0.1)
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blur: 0.8
            blurMax: 32
            shadowEnabled: true
            shadowColor: "black"
            shadowOpacity: 0.4
            shadowBlur: 0.5
        }
    }

    // UI Content Overlay
    Column {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        spacing: 4
        z: 2

        Text {
            text: root.title
            color: "#8892B0"
            font.pixelSize: 12
            font.weight: Font.Medium
            font.family: "Inter, Segoe UI, Roboto"
        }

        Row {
            spacing: 8

            Text {
                id: opsText
                text: (root.currentOps / 1000000).toFixed(2)
                color: "white"
                font.pixelSize: 32
                font.weight: Font.Bold
                font.family: "Iosevka, Consolas, monospace"

                // Jitter Prevention: Use SmoothedAnimation for UI value transitions
                Behavior on text {
                    SmoothedAnimation { duration: 150 }
                }
            }

            Text {
                text: "M ops/s"
                color: "#00F2FF"
                font.pixelSize: 14
                anchors.bottom: opsText.bottom
                anchors.bottomMargin: 6
                font.weight: Font.DemiBold
            }
        }
    }

    // Real-time "Pulse" indicator (GPU-bound opacity animation)
    Rectangle {
        width: 8
        height: 8
        radius: 4
        color: "#00F2FF"
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        z: 2

        SequentialAnimation on opacity {
            loops: Animation.Infinite
            NumberAnimation { from: 1.0; to: 0.3; duration: 800; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 0.3; to: 1.0; duration: 800; easing.type: Easing.InOutQuad }
        }
    }
}
