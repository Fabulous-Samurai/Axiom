import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import "components"

/*
 * AXIOM System Flow Dashboard
 *
 * ┌──────────────────────────────────────────────────────────┐
 * │  AXIOM SYSTEM FLOW                    ● REC   ⟳ Reset  │
 * ├──────────────────────────────────────────────────────────┤
 * │                                                          │
 * │  [Input] ──→ [Pipe] ──→ [Queue] ──→ [Proc] ──→ [Plot] │
 * │                                                          │
 * ├──────────────────────────────────────────────────────────┤
 * │  CPU ████░░ 64%  │  RAM ██████░░ 78%  │  GPU ███░░ 34% │
 * ├──────────────────────────────────────────────────────────┤
 * │  [Throughput Chart]          [Latency Chart]             │
 * └──────────────────────────────────────────────────────────┘
 */

Window {
    id: dashboard
    visible: true
    width: 1280
    height: 720

    // Dashboard Manager (C++ backend)
    property var manager: dashboardManager

    // Visual
    color: "#050505" // Darker base

    // Background Radial Highlight (Sprint 3)
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0F172A" }
            GradientStop { position: 1.0; color: "#020617" }
        }

        // Background Grid Mesh
        Repeater {
            model: parent.width / 40
            Rectangle {
                x: index * 40; y: 0
                width: 1; height: parent.height
                color: Qt.rgba(1, 1, 1, 0.03)
            }
        }
        Repeater {
            model: parent.height / 40
            Rectangle {
                x: 0; y: index * 40
                width: parent.width; height: 1
                color: Qt.rgba(1, 1, 1, 0.03)
            }
        }
    }

    property bool showDebugOverlay: false
    Shortcut {
        sequence: "Ctrl+Shift+D"
        onActivated: showDebugOverlay = !showDebugOverlay
    }

    // ═══════════════════════════════════════════════════════════════
    // HEADER
    // ═══════════════════════════════════════════════════════════════

    Rectangle {
        id: header
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 56
        color: Qt.rgba(0.06, 0.06, 0.08, 0.95)
        border.color: "#1E293B"
        border.width: 0

        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: "#1E293B"
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            // Logo + Title
            RowLayout {
                spacing: 12

                // AXIOM Logo pulse
                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: "#3B82F6"

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { to: 0.3; duration: 1500 }
                        NumberAnimation { to: 1.0; duration: 1500 }
                    }
                }

                Text {
                    text: "AXIOM"
                    font.pixelSize: 16
                    font.weight: Font.Bold
                    font.letterSpacing: 3
                    color: "#F8FAFC"
                }

                Text {
                    text: "SYSTEM FLOW"
                    font.pixelSize: 16
                    font.weight: Font.Light
                    font.letterSpacing: 2
                    color: "#64748B"
                }
            }

            Item { Layout.fillWidth: true }

            // Status badges
            RowLayout {
                spacing: 16

                // Active stages
                Row {
                    spacing: 6
                    Text {
                        text: "●"
                        color: "#22C55E"
                        font.pixelSize: 10
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: (manager ? manager.activeStageCount : 0) + " Active"
                        font.pixelSize: 12
                        color: "#94A3B8"
                        font.family: "JetBrains Mono"
                    }
                }

                // Total throughput
                Text {
                    text: formatThroughput(manager ? manager.totalThroughput : 0)
                    font.pixelSize: 12
                    color: "#94A3B8"
                    font.family: "JetBrains Mono"
                }

                // Record button
                Rectangle {
                    width: 70
                    height: 28
                    radius: 4
                    color: manager && manager.recording
                           ? Qt.rgba(0.94, 0.27, 0.27, 0.2)
                           : Qt.rgba(0.23, 0.51, 0.96, 0.2)
                    border.color: manager && manager.recording
                                  ? "#EF4444" : "#3B82F6"

                    Row {
                        anchors.centerIn: parent
                        spacing: 6

                        Rectangle {
                            width: 8; height: 8; radius: 4
                            color: manager && manager.recording ? "#EF4444" : "#3B82F6"
                            anchors.verticalCenter: parent.verticalCenter

                            SequentialAnimation on opacity {
                                running: manager && manager.recording
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.3; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }

                        Text {
                            text: manager && manager.recording ? "REC" : "REC"
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            color: manager && manager.recording ? "#EF4444" : "#3B82F6"
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (manager.recording) {
                                manager.stopRecording()
                            } else {
                                manager.startRecording()
                            }
                        }
                    }
                }

                // Reset button
                Rectangle {
                    width: 28; height: 28; radius: 4
                    color: resetMouse.containsMouse
                           ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "⟳"
                        font.pixelSize: 16
                        color: "#64748B"
                    }

                    MouseArea {
                        id: resetMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: manager.resetCounters()
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // FLOW TOPOLOGY VIEW
    // ═══════════════════════════════════════════════════════════════

    Rectangle {
        id: topologyContainer
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 220
        color: "transparent"

        // Glass panel
        Rectangle {
            anchors.fill: parent
            anchors.margins: 16
            radius: 12
            color: Qt.rgba(0.06, 0.08, 0.12, 0.6)
            border.color: "#1E293B"
            border.width: 1

            // Section title
            Text {
                x: 16; y: 12
                text: "PIPELINE TOPOLOGY"
                font.pixelSize: 10
                font.weight: Font.Medium
                font.letterSpacing: 1.5
                color: "#475569"
            }

            // Flow nodes
            Item {
                id: flowArea
                anchors.fill: parent
                anchors.topMargin: 48
                anchors.bottomMargin: 24
                anchors.leftMargin: 48
                anchors.rightMargin: 48

                // Dynamic node placement
                Repeater {
                    id: nodeRepeater
                    model: manager ? manager.stages : []

                    FlowNode {
                        property int nodeIndex: index
                        property int totalNodes: manager ? manager.stages.length : 1

                        x: (flowArea.width - nodeWidth) *
                           (nodeIndex / Math.max(1, totalNodes - 1))
                        y: (flowArea.height - nodeHeight) / 2

                        nodeId: modelData.id
                        nodeName: modelData.name
                        state: modelData.stateName
                        messagesPerSecond: modelData.messagesPerSecond
                        bytesPerSecond: modelData.bytesPerSecond
                        avgLatencyUs: modelData.avgLatencyUs
                        queueFillRatio: modelData.queueFillRatio
                        errorMessage: modelData.errorMessage

                        onNodeClicked: {
                            detailPanel.stageId = nodeId
                            detailPanel.stageName = nodeName
                            detailPanel.messagesPerSecond = messagesPerSecond
                            detailPanel.avgLatencyUs = avgLatencyUs
                            detailPanel.queueFillRatio = queueFillRatio
                            detailPanel.errorMessage = errorMessage
                            detailPanel.visible = true
                        }
                    }
                }

                // Dynamic links between nodes
                Repeater {
                    model: manager ? manager.links : []

                    FlowLink {
                        property var sourceNode: findNode(modelData.sourceId)
                        property var targetNode: findNode(modelData.targetId)

                        startPoint: sourceNode
                            ? Qt.point(sourceNode.x + sourceNode.width,
                                       sourceNode.y + sourceNode.height / 2)
                            : Qt.point(0, 0)

                        endPoint: targetNode
                            ? Qt.point(targetNode.x,
                                       targetNode.y + targetNode.height / 2)
                            : Qt.point(0, 0)

                        property real targetQueueFill: targetNode ? targetNode.queueFillRatio : 0

                        throughputRatio: modelData.throughputRatio
                        dropRate: modelData.dropRate
                        active: modelData.active
                        backpressure: targetQueueFill > 0.8
                        linkColor: "#3B82F6"

                        anchors.fill: parent
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // SYSTEM TELEMETRY BAR
    // ═══════════════════════════════════════════════════════════════

    Rectangle {
        id: telemetryBar
        anchors.top: topologyContainer.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
        color: "transparent"

        Rectangle {
            anchors.fill: parent
            anchors.margins: 16
            anchors.topMargin: 0
            radius: 12
            color: Qt.rgba(0.06, 0.08, 0.12, 0.6)
            border.color: "#1E293B"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 24

                // CPU
                TelemetryGauge {
                    label: "CPU"
                    value: manager ? manager.systemTelemetry.cpuUsage : 0
                    unit: "%"
                    maxValue: 100
                    color: "#3B82F6"
                    Layout.fillWidth: true
                }

                // Separator
                Rectangle { width: 1; Layout.fillHeight: true; color: "#1E293B" }

                // RAM
                TelemetryGauge {
                    label: "RAM"
                    value: manager ? manager.systemTelemetry.ramUsage : 0
                    unit: "%"
                    subtitle: manager ? formatBytes(manager.systemTelemetry.ramUsed)
                              + " / " + formatBytes(manager.systemTelemetry.ramTotal) : ""
                    maxValue: 100
                    color: "#22C55E"
                    Layout.fillWidth: true
                }

                Rectangle { width: 1; Layout.fillHeight: true; color: "#1E293B" }

                // GPU
                TelemetryGauge {
                    label: "GPU"
                    value: manager && manager.systemTelemetry.gpuAvailable
                           ? manager.systemTelemetry.gpuUsage : 0
                    unit: "%"
                    subtitle: manager && manager.systemTelemetry.gpuAvailable
                              ? manager.systemTelemetry.gpuTemp.toFixed(0) + "°C" : "N/A"
                    maxValue: 100
                    color: "#EAB308"
                    enabled: manager ? manager.systemTelemetry.gpuAvailable : false
                    Layout.fillWidth: true
                }

                Rectangle { width: 1; Layout.fillHeight: true; color: "#1E293B" }

                // IPC Throughput
                TelemetryGauge {
                    label: "IPC"
                    value: manager ? manager.systemTelemetry.ipcBps / 1048576 : 0
                    unit: "MB/s"
                    maxValue: 1000
                    color: "#8B5CF6"
                    Layout.fillWidth: true
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // LIVE CHARTS
    // ═══════════════════════════════════════════════════════════════

    RowLayout {
        anchors.top: telemetryBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 16
        anchors.topMargin: 0
        spacing: 16

        // Throughput over time
        Rectangle {
            Layout.fillWidth: true
            height: 280
            radius: 12
            color: Qt.rgba(0.06, 0.08, 0.12, 0.6)
            border.color: "#1E293B"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                Text {
                    text: "THROUGHPUT"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    color: "#475569"
                }

                AXChart {
                    id: throughputChart
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    type: "area"
                    primaryColor: "#3B82F6"
                    backgroundColor: "#0F172A"
                    warningThreshold: 5000
                    criticalThreshold: 8000
                    gridColor: "#1E293B"
                    showLegend: false

                    property var dataPoints: []
                    data: dataPoints

                    Timer {
                        interval: 1000
                        running: true
                        repeat: true
                        onTriggered: {
                            var tp = manager ? manager.totalThroughput : 0
                            var newPoints = throughputChart.dataPoints.slice()
                            newPoints.push({
                                value: tp,
                                label: new Date().toLocaleTimeString('en-US', { second: '2-digit' })
                            })
                            if (newPoints.length > 60) newPoints.shift()
                            throughputChart.dataPoints = newPoints
                        }
                    }
                }
            }
        }

        // Latency over time
        Rectangle {
            Layout.fillWidth: true
            height: 280
            radius: 12
            color: Qt.rgba(0.06, 0.08, 0.12, 0.6)
            border.color: "#1E293B"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                Text {
                    text: "LATENCY (P99)"
                    font.pixelSize: 10
                    font.letterSpacing: 1.5
                    color: "#475569"
                }

                AXChart {
                    id: latencyChart
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    type: "area"
                    primaryColor: "#EAB308"
                    backgroundColor: "#0F172A"
                    gridColor: "#1E293B"
                    showLegend: false
                    warningThreshold: 800
                    criticalThreshold: 1500

                    property var dataPoints: []
                    data: dataPoints

                    Timer {
                        interval: 1000
                        running: true
                        repeat: true
                        onTriggered: {
                            var lat = manager ? manager.totalLatency : 0
                            var newPoints = latencyChart.dataPoints.slice()
                            newPoints.push({
                                value: lat,
                                label: new Date().toLocaleTimeString('en-US', { second: '2-digit' })
                            })
                            if (newPoints.length > 60) newPoints.shift()
                            latencyChart.dataPoints = newPoints
                        }
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // HELPER FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    function findNode(stageId) {
        for (var i = 0; i < nodeRepeater.count; i++) {
            var item = nodeRepeater.itemAt(i)
            if (item && item.nodeId === stageId) return item
        }
        return null
    }

    function formatThroughput(value) {
        if (value >= 1000000) return (value / 1000000).toFixed(1) + "M msg/s"
        if (value >= 1000) return (value / 1000).toFixed(1) + "K msg/s"
        return value.toFixed(0) + " msg/s"
    }

    function formatBytes(bytes) {
        if (bytes >= 1073741824) return (bytes / 1073741824).toFixed(1) + " GB"
        if (bytes >= 1048576) return (bytes / 1048576).toFixed(0) + " MB"
        if (bytes >= 1024) return (bytes / 1024).toFixed(0) + " KB"
        return bytes + " B"
    }

    // ═══════════════════════════════════════════════════════════════
    // DIAGNOSTIC OVERLAYS
    // ═══════════════════════════════════════════════════════════════

    DetailPanel {
        id: detailPanel
        visible: false

        // History Track Timer (Sprint 3)
        Timer {
            interval: 2000
            running: detailPanel.visible
            repeat: true
            onTriggered: {
                if (!detailPanel.stageId) return

                var currentHistory = detailPanel.history || []
                var newHistory = currentHistory.slice()
                newHistory.unshift({
                    time: new Date().toLocaleTimeString('en-US', { hour12: false }),
                    throughput: detailPanel.messagesPerSecond,
                    latency: detailPanel.avgLatencyUs
                })

                if (newHistory.length > 10) newHistory.pop()
                detailPanel.history = newHistory
            }
        }
    }

    // Performance Overlay (Hidden/Sprint 3)
    Rectangle {
        visible: showDebugOverlay
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 200
        height: 100
        color: Qt.rgba(0.06, 0.08, 0.12, 0.9)
        radius: 8
        border.color: "#3B82F6"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 4
            Text { text: "DASHBOARD DEBUG"; font.pixelSize: 10; color: "#3B82F6"; font.weight: Font.Bold; font.letterSpacing: 1 }
            Rectangle { Layout.fillWidth: true; height: 1; color: "#1E293B" }
            Text { text: "Frame Time: 16.4ms"; font.pixelSize: 11; color: "#94A3B8"; font.family: "JetBrains Mono" }
            Text { text: "App Memory: 142MB"; font.pixelSize: 11; color: "#94A3B8"; font.family: "JetBrains Mono" }
            Text { text: "QML Objects: 482"; font.pixelSize: 11; color: "#94A3B8"; font.family: "JetBrains Mono" }
        }
    }
}
