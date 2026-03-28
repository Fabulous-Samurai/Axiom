import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════
    property string stageId: ""
    property string stageName: "Unknown Stage"
    property real throughput: 0
    property real latency: 0
    property real queueSaturation: 0  // 0.0 to 1.0
    property string type: "compute"   // "compute", "storage", "ai"
    
    signal clicked()
    
    // EMA Smoothing State
    property real smoothSaturation: 0
    property real emaAlpha: 0.2
    
    onQueueSaturationChanged: {
        smoothSaturation = (queueSaturation * emaAlpha) + (smoothSaturation * (1.0 - emaAlpha))
    }
    
    // Visual Configuration
    width: 140
    height: 100
    
    // ═══════════════════════════════════════════════════════════════
    // COMPUTED STATE
    // ═══════════════════════════════════════════════════════════════
    
    readonly property color severityColor: {
        if (smoothSaturation > 0.8) return "#EF4444" // CRITICAL
        if (smoothSaturation > 0.5) return "#F59E0B" // WARNING
        return "#22C55E" // HEALTHY (Green)
    }
    
    // Vibration Logic (Backpressure)
    property real vibrationOffset: 0
    Timer {
        id: vibrationTimer
        interval: 30
        repeat: true
        running: smoothSaturation > 0.8
        onTriggered: {
            vibrationOffset = (Math.random() - 0.5) * 4
        }
        onRunningChanged: {
            if (!running) vibrationOffset = 0
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // UI COMPONENTS
    // ═══════════════════════════════════════════════════════════════
    
    Rectangle {
        id: body
        anchors.fill: parent
        anchors.leftMargin: vibrationOffset
        anchors.rightMargin: -vibrationOffset
        
        color: "#1E293B"
        radius: 8
        border.color: severityColor
        border.width: smoothSaturation > 0.8 ? 2 : 1
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 4
            
            // Header
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: stageName.toUpperCase()
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.family: "JetBrains Mono"
                    color: "#F8FAFC"
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                Rectangle {
                    width: 8; height: 8; radius: 4
                    color: severityColor
                }
            }
            
            Item { Layout.fillHeight: true }
            
            // Metrics
            Column {
                spacing: 2
                Text {
                    text: "TP: " + formatThroughput(throughput)
                    font.pixelSize: 10
                    font.family: "JetBrains Mono"
                    color: "#94A3B8"
                }
                Text {
                    text: "LAT: " + latency.toFixed(2) + "ms"
                    font.pixelSize: 10
                    font.family: "JetBrains Mono"
                    color: "#94A3B8"
                }
            }
            
            // Saturation Bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 4
                color: "#334155"
                radius: 2
                
                Rectangle {
                    width: parent.width * smoothSaturation
                    height: parent.height
                    radius: 2
                    color: severityColor
                    Behavior on width { NumberAnimation { duration: 200 } }
                }
            }
        }
        
        // Hover effect
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: body.opacity = 1.0
            onExited: body.opacity = 0.9
            onClicked: root.clicked()
        }
        opacity: 0.9
    }
    
    function formatThroughput(val) {
        if (val >= 1000000) return (val / 1000000).toFixed(1) + "M/s"
        if (val >= 1000) return (val / 1000).toFixed(1) + "K/s"
        return val.toFixed(0) + "/s"
    }
}
