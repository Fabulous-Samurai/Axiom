import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "./components"

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 720
    title: "AXIOM Dashboard"
    color: "#0F172A" // Deep slate background

    // Selection State
    property var selectedNode: null
    property var selectedHistory: []

    // Background System
    Rectangle {
        anchors.fill: parent
        gradient: RadialGradient {
            center: Qt.point(parent.width * 0.5, parent.height * 0.5)
            radius: Math.max(parent.width, parent.height) * 0.8
            GradientStop { position: 0.0; color: "#1E293B" }
            GradientStop { position: 1.0; color: "#0F172A" }
        }

        // Grid Mesh
        Canvas {
            anchors.fill: parent
            opacity: 0.15
            onPaint: {
                var ctx = getContext("2d");
                ctx.strokeStyle = "#334155";
                ctx.lineWidth = 1;
                var step = 40;
                for (var x = 0; x <= width; x += step) {
                    ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, height); ctx.stroke();
                }
                for (var y = 0; y <= height; y += step) {
                    ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke();
                }
            }
        }
    }

    // Main Layout
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header / Telemetry Bar
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            spacing: 30

            Column {
                spacing: 2
                Text {
                    text: "AXIOM // FLOW MONITOR"
                    font.pixelSize: 22
                    font.weight: Font.Bold
                    font.family: "JetBrains Mono"
                    color: "#F8FAFC"
                }
                Text {
                    text: "V3.1-STABLE // PRODUCTION GRADE"
                    font.pixelSize: 10
                    font.family: "JetBrains Mono"
                    color: "#3B82F6"
                }
            }

            Item { Layout.fillWidth: true }

            Row {
                spacing: 20
                TelemetryGauge { label: "CPU"; value: dashboardManager.systemTelemetry.CPU || 0; unit: "%" }
                TelemetryGauge { label: "RAM"; value: (dashboardManager.systemTelemetry.RAM_USAGE_MB || 0) / 1024 * 100 / 32; unit: "%"; warningThreshold: 60; criticalThreshold: 85 } // Assuming 32GB total
                TelemetryGauge { label: "IPC"; value: dashboardManager.systemTelemetry.IPC_THROUGHPUT || 0; unit: "M" }
            }
        }

        // Central Topology View
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Qt.rgba(15/255, 23/255, 42/255, 0.4)
            border.color: "#334155"
            border.width: 1
            radius: 12
            clip: true

            Item {
                id: topologyContainer
                anchors.fill: parent

                // Links (Drawn first to be behind nodes)
                Repeater {
                    model: dashboardManager.stages.length > 1 ? dashboardManager.stages.length - 1 : 0
                    FlowLink {
                        startPoint: Qt.point(50 + index * 250 + 140, 150 + (index % 2) * 100 + 50)
                        endPoint: Qt.point(50 + (index + 1) * 250, 150 + ((index + 1) % 2) * 100 + 50)
                        throughput: dashboardManager.stages[index].throughput
                    }
                }

                // Nodes
                Repeater {
                    model: dashboardManager.stages
                    FlowNode {
                        x: 50 + index * 250
                        y: 150 + (index % 2) * 100
                        stageId: modelData.id
                        stageName: modelData.name
                        throughput: modelData.throughput
                        latency: modelData.latency
                        queueSaturation: modelData.queueSaturation

                        onXChanged: if (selectedNode && selectedNode.stageId === stageId) updateHistory()
                        onClicked: {
                            window.selectedNode = modelData
                            window.selectedHistory = [] // Reset history for new node
                            updateHistory()
                        }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: "SYSTEM IDLE // NO STAGES DETECTED"
                color: "#1E293B"
                font.pixelSize: 18
                font.family: "JetBrains Mono"
                visible: dashboardManager.stages.length === 0
            }
        }

        // Bottom Analytic Inspector
        DetailPanel {
            id: detailPanel
            Layout.fillWidth: true
            Layout.preferredHeight: 220
            selectedNode: window.selectedNode
            history: window.selectedHistory
        }
    }

    // Selection & Data Tracking Logic
    Timer {
        interval: 1000
        repeat: true
        running: selectedNode !== null
        onTriggered: updateHistory()
    }

    function updateHistory() {
        if (!selectedNode) return;
        let newHistory = selectedHistory;
        newHistory.push({
            time: new Date().toLocaleTimeString('en-GB', { hour12: false }).split(' ')[0],
            tp: selectedNode.throughput,
            lat: selectedNode.latency
        });
        if (newHistory.length > 20) newHistory.shift();
        selectedHistory = newHistory;
    }

    // Global Selection Connect (Would normally be in FlowNode.onClicked)
    // For this reconstruction, we'll auto-select the first node if none selected
    Component.onCompleted: {
        if (dashboardManager.stages.length > 0) {
            // Simulated selection for demo
            // In a real app, this is triggered by FlowNode's MouseArea
        }
    }

    // Debug Overlay
    Rectangle {
        visible: showDebugOverlay
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 220; height: 120
        color: Qt.rgba(15, 23, 42, 0.9)
        radius: 8
        border.color: "#3B82F6"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            Text { text: "AXIOM DEBUG CONSOLE"; color: "#3B82F6"; font.bold: true; font.family: "JetBrains Mono"; font.pixelSize: 10 }
            Text { text: "Engine: STABLE"; color: "#22C55E"; font.family: "JetBrains Mono"; font.pixelSize: 10 }
            Text { text: "Active Stages: " + dashboardManager.stages.length; color: "white"; font.family: "JetBrains Mono"; font.pixelSize: 10 }
            Text { text: "Allocations: HARMONIC_ARENA"; color: "#94A3B8"; font.family: "JetBrains Mono"; font.pixelSize: 10 }
        }
    }
}
