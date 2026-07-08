import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../theme"
import "../components"

Item {
    id: root
    anchors.fill: parent

    property int currentResIndex: 1
    property bool useGPU: false
    property real latencyMs: 1.5
    property real cpuLatency: 1.5
    property real gpuLatency: 0.09

    function getResolution(index) {
        var res = [64, 128, 256, 512, 1024]
        return res[index]
    }

    Connections {
        target: engineBridge
        function onGraphicsRendered(imagePath) {
            graphicsCanvas.source = "" // Reset to force reload if same path
            graphicsCanvas.source = "file:///" + imagePath
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Spacing.xlarge
        clip: true

        Column {
            width: parent.width
            spacing: Spacing.large

            Text {
                text: "Graphics Visualizer"
                font.family: Typography.heading
                font.pixelSize: Typography.h2
                color: Colors.textPrimary
                font.weight: Typography.semibold
            }

            ExpressionInput {
                id: graphicsInput
                placeholder: "sin(x) * cos(y)"
                mode: "plot"
                onAccepted: {
                    engineBridge.evaluateGraphics(text, getResolution(currentResIndex))
                }
            }

            Row {
                spacing: Spacing.medium
                Repeater {
                    model: ["64×64", "128×128", "256×256", "512×512", "1024×1024"]
                    ActionButton {
                        text: modelData
                        variant: index === currentResIndex ? "primary" : "ghost"
                        onClicked: currentResIndex = index
                    }
                }
            }

            Row {
                spacing: Spacing.medium

                Rectangle {
                    width: 100; height: 36; radius: Spacing.radiusMedium
                    color: !useGPU ? Colors.primary : "transparent"
                    border.color: !useGPU ? "transparent" : Colors.border; border.width: 1
                    Text { anchors.centerIn: parent; text: "CPU"; color: !useGPU ? Colors.surface : Colors.textPrimary; font.weight: Typography.medium }
                    MouseArea { anchors.fill: parent; onClicked: useGPU = false }
                }
                Rectangle {
                    width: 120; height: 36; radius: Spacing.radiusMedium
                    color: useGPU ? Colors.primary : "transparent"
                    border.color: useGPU ? "transparent" : Colors.border; border.width: 1
                    Text { anchors.centerIn: parent; text: "GPU (Vulkan)"; color: useGPU ? Colors.surface : Colors.textPrimary; font.weight: Typography.medium }
                    MouseArea { anchors.fill: parent; onClicked: useGPU = true }
                }
            }

            GraphicsCanvas {
                id: graphicsCanvas
                width: 512; height: 512
                source: ""
                expression: graphicsInput.text
            }

            GlassPanel {
                width: parent.width; height: 100
                Row {
                    anchors.fill: parent; anchors.margins: Spacing.mediumLarge
                    spacing: Spacing.xlarge
                    Column {
                        Text { text: "Latency"; color: Colors.textSecondary }
                        Text {
                            text: (useGPU ? gpuLatency : cpuLatency).toFixed(2) + " ms"
                            font.pixelSize: Typography.metric
                            font.weight: Typography.bold
                            color: useGPU ? Colors.success : Colors.info
                        }
                    }
                    Column {
                        visible: useGPU
                        Text { text: "Speedup"; color: Colors.textSecondary }
                        Text {
                            text: (cpuLatency / gpuLatency).toFixed(1) + "x ⚡"
                            font.pixelSize: Typography.metric
                            color: Colors.success
                        }
                    }
                }
            }
        }
    }
}
