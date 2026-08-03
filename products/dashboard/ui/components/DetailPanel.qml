import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    property string stageId: ""
    property string stageName: "Select a Stage"
    property real messagesPerSecond: 0
    property real avgLatencyUs: 0
    property real queueFillRatio: 0
    property string errorMessage: ""
    property var history: [] // Array of {throughput, latency, time}

    // ═══════════════════════════════════════════════════════════════
    // VISUAL
    // ═══════════════════════════════════════════════════════════════

    width: 300
    height: parent.height - 40
    anchors.right: parent.right
    anchors.rightMargin: visible ? 20 : -width
    anchors.verticalCenter: parent.verticalCenter
    radius: 12

    // Glass Background
    color: Qt.rgba(0.06, 0.08, 0.12, 0.95)
    border.color: "#1E293B"
    border.width: 1

    Behavior on anchors.rightMargin {
        NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
    }

    // Header Highlight
    Rectangle {
        width: parent.width
        height: 48
        radius: parent.radius
        color: Qt.rgba(1, 1, 1, 0.05)
        anchors.top: parent.top

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16

            Text {
                text: stageName.toUpperCase()
                font.pixelSize: 12
                font.weight: Font.Bold
                font.letterSpacing: 1.2
                color: "#F8FAFC"
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "✕"
                font.pixelSize: 16
                color: "#64748B"

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.visible = false
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 64
        anchors.margins: 20
        spacing: 24

        // Stats Grid
        GridLayout {
            columns: 2
            rowSpacing: 16
            columnSpacing: 16
            Layout.fillWidth: true

            StatEntry { label: "Throughput"; value: formatThroughput(messagesPerSecond) }
            StatEntry { label: "Latency"; value: formatLatency(avgLatencyUs) }
            StatEntry { label: "Queue Load"; value: (queueFillRatio * 100).toFixed(1) + "%" }
            StatEntry { label: "Status"; value: errorMessage === "" ? "HEALTHY" : "ERROR"; color: errorMessage === "" ? "#22C55E" : "#EF4444" }
        }

        // Actionable area
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#1E293B"
        }

        // Analytic Metric History (Sprint 3)
        ColumnLayout {
            spacing: 8
            Layout.fillWidth: true

            Text {
                text: "HISTORICAL TREND (LAST 10)"
                font.pixelSize: 10
                font.weight: Font.Bold
                color: "#475569"
            }

            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: "#0F172A"
                radius: 4
                clip: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    // Table Header
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "TIME"; font.pixelSize: 9; color: "#64748B"; Layout.preferredWidth: 60 }
                        Text { text: "TPUT"; font.pixelSize: 9; color: "#64748B"; Layout.preferredWidth: 80 }
                        Text { text: "LATENCY"; font.pixelSize: 9; color: "#64748B"; Layout.fillWidth: true }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#1E293B" }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.history
                        interactive: false
                        delegate: RowLayout {
                            width: parent.width
                            Text { text: modelData.time || "--:--:--"; font.pixelSize: 10; font.family: "JetBrains Mono"; color: "#94A3B8"; Layout.preferredWidth: 60 }
                            Text { text: formatThroughput(modelData.throughput || 0); font.pixelSize: 10; font.family: "JetBrains Mono"; color: "#3B82F6"; Layout.preferredWidth: 80 }
                            Text { text: formatLatency(modelData.latency || 0); font.pixelSize: 10; font.family: "JetBrains Mono"; color: "#EAB308"; Layout.fillWidth: true }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            spacing: 8
            Layout.fillWidth: true

            Text {
                text: "DIAGNOSTIC LOGS"
                font.pixelSize: 10
                font.weight: Font.Bold
                color: "#475569"
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#0F172A"
                radius: 4
                clip: true

                ListView {
                    anchors.fill: parent
                    anchors.margins: 8
                    model: errorMessage === "" ? ["Ready for diagnostics...", "Connection stable", "Monitoring node: " + stageId] : [errorMessage, "Investigation required"]
                    delegate: Text {
                        text: "> " + modelData
                        font.pixelSize: 11
                        font.family: "JetBrains Mono"
                        color: index === 0 && errorMessage !== "" ? "#EF4444" : "#94A3B8"
                        width: parent.width
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }

    // Small component for stats
    component StatEntry : ColumnLayout {
        property string label: ""
        property string value: ""
        property color color: "#F8FAFC"

        spacing: 4

        Text {
            text: label
            font.pixelSize: 9
            font.weight: Font.Bold
            color: "#64748B"
        }
        Text {
            text: value
            font.pixelSize: 14
            font.weight: Font.Medium
            font.family: "JetBrains Mono"
            color: parent.color
        }
    }

    function formatThroughput(v) {
        if (v >= 1000) return (v/1000).toFixed(1) + "K msg/s"
        return v.toFixed(0) + " msg/s"
    }

    function formatLatency(v) {
        if (v >= 1000) return (v/1000).toFixed(1) + "ms"
        return v.toFixed(0) + "μs"
    }
}
