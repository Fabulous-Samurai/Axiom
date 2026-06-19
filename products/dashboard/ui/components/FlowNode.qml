import QtQuick 2.15
import QtQuick.Layouts 1.15

/*
 * FlowNode — Simatic-style pipeline stage indicator
 *
 * Görünüm:
 * ┌──────────────────────────┐
 * │  ◉ INPUT SOURCE          │
 * │  ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔  │
 * │  12.4K msg/s             │
 * │  █████████░░░░ 78%       │
 * │  Latency: 0.2ms          │
 * └──────────────────────────┘
 */

Rectangle {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    property string nodeId: ""
    property string nodeName: "Stage"
    property string state: "offline"     // offline, idle, active, blocked, error
    property real messagesPerSecond: 0
    property real bytesPerSecond: 0
    property real peakMessagesPerSecond: 0
    property real avgLatencyUs: 0
    property real queueFillRatio: 0
    property real uptimeSeconds: 0
    property string errorMessage: ""
    property real shakeOffset: 0

    // Visual
    property real nodeWidth: 200
    property real nodeHeight: 140

    // Phase 6.1: Health & Vibration
    property real healthScore: Math.max(0, 1.0 - (queueFillRatio * 0.7 + (avgLatencyUs > 200 ? 0.3 : 0)))
    property color healthColor: {
        if (healthScore > 0.8) return "#22C55E" // Green
        if (healthScore > 0.4) return "#F59E0B" // Orange/Yellow
        return "#EF4444" // Red
    }

    // ═══════════════════════════════════════════════════════════════
    // COMPUTED
    // ═══════════════════════════════════════════════════════════════

    readonly property color stateColor: {
        switch (state) {
            case "active":  return "#22C55E"    // Yeşil
            case "idle":    return "#3B82F6"    // Mavi
            case "blocked": return "#EAB308"    // Sarı
            case "error":   return "#EF4444"    // Kırmızı
            default:        return "#475569"    // Gri (offline)
        }
    }

    readonly property string stateIcon: {
        switch (state) {
            case "active":  return "◉"
            case "idle":    return "◎"
            case "blocked": return "⊘"
            case "error":   return "✖"
            default:        return "○"
        }
    }

    readonly property string throughputText: {
        if (messagesPerSecond >= 1000000) {
            return (messagesPerSecond / 1000000).toFixed(1) + "M msg/s"
        } else if (messagesPerSecond >= 1000) {
            return (messagesPerSecond / 1000).toFixed(1) + "K msg/s"
        }
        return messagesPerSecond.toFixed(0) + " msg/s"
    }

    readonly property string latencyText: {
        if (avgLatencyUs >= 1000) {
            return (avgLatencyUs / 1000).toFixed(1) + "ms"
        }
        return avgLatencyUs.toFixed(0) + "μs"
    }

    // ═══════════════════════════════════════════════════════════════
    // VISUAL
    // ═══════════════════════════════════════════════════════════════

    width: nodeWidth
    height: nodeHeight
    radius: 8
    antialiasing: true

    // Glass background
    color: Qt.rgba(0.12, 0.15, 0.2, 0.95)

    // Subtle gradient for glass effect (P3.2)
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        opacity: 0.1
        gradient: Gradient {
            GradientStop { position: 0.0; color: "white" }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    border.color: errorMessage !== "" ? "#EF4444" : healthColor
    border.width: 1.5

    // Backpressure Vibration (Sprint 2)
    SequentialAnimation on x {
        running: queueFillRatio > 0.8
        loops: Animation.Infinite
        NumberAnimation { from: root.x - 1; to: root.x + 1; duration: 40; easing.type: Easing.Linear }
        NumberAnimation { from: root.x + 1; to: root.x - 1; duration: 40; easing.type: Easing.Linear }
    }

    // State glow pulse (active durumda)
    Rectangle {
        id: glowPulse
        anchors.fill: parent
        anchors.margins: -4
        radius: parent.radius + 4
        color: "transparent"
        border.color: stateColor
        border.width: 2
        opacity: 0
        visible: state === "active"

        SequentialAnimation on opacity {
            running: state === "active"
            loops: Animation.Infinite
            NumberAnimation { to: 0.9; duration: 800; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.2; duration: 800; easing.type: Easing.InOutSine }
        }
    }

    // Error shake animation
    SequentialAnimation {
        id: errorShake
        running: state === "error"
        loops: 3

        NumberAnimation { target: root; property: "shakeOffset"; to: 5; duration: 50 }
        NumberAnimation { target: root; property: "shakeOffset"; to: -5; duration: 50 }
        NumberAnimation { target: root; property: "shakeOffset"; to: 0; duration: 50 }
    }

    // ═══════════════════════════════════════════════════════════════
    // CONTENT
    // ═══════════════════════════════════════════════════════════════

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        // Header: State icon + Name
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: stateIcon
                font.pixelSize: 14
                color: stateColor
            }

            Text {
                Layout.fillWidth: true
                text: nodeName.toUpperCase()
                font.pixelSize: 11
                font.weight: Font.Bold
                font.letterSpacing: 1.2
                color: "#E2E8F0"
                elide: Text.ElideRight
            }
        }

        // Separator
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(stateColor.r, stateColor.g, stateColor.b, 0.3)
        }

        // Throughput
        Text {
            text: throughputText
            font.pixelSize: 18
            font.weight: Font.Bold
            color: state === "active" ? "#F8FAFC" : "#64748B"
            font.family: "JetBrains Mono"
        }

        // Queue bar (varsa)
        Item {
            Layout.fillWidth: true
            height: 8
            visible: queueFillRatio > 0

            Rectangle {
                anchors.fill: parent
                radius: 4
                color: "#1E293B"
            }

            Rectangle {
                width: parent.width * queueFillRatio
                height: parent.height
                radius: 4
                color: queueFillRatio > 0.9 ? "#EF4444" :
                       queueFillRatio > 0.7 ? "#EAB308" : stateColor

                Behavior on width {
                    NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                }
            }
        }

        // Latency
        RowLayout {
            spacing: 4

            Text {
                text: "Latency:"
                font.pixelSize: 10
                color: "#64748B"
            }

            Text {
                text: latencyText
                font.pixelSize: 10
                font.weight: Font.Medium
                color: avgLatencyUs > 1000 ? "#EAB308" :
                       avgLatencyUs > 5000 ? "#EF4444" : "#94A3B8"
                font.family: "JetBrains Mono"
            }
        }

        // Error message
        Text {
            visible: state === "error" && errorMessage !== ""
            Layout.fillWidth: true
            text: "⚠ " + errorMessage
            font.pixelSize: 9
            color: "#EF4444"
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
        }
    }

    // Hover effect
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: root.scale = 1.03
        onExited: root.scale = 1.0
        onClicked: root.nodeClicked()
    }

    Behavior on scale {
        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
    }

    transform: Translate { x: shakeOffset }

    signal nodeClicked()
}
