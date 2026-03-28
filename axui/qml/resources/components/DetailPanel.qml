import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════
    property var selectedNode: null
    property var history: [] // List of {tp, lat, time}
    
    color: "#1E293B"
    radius: 8
    border.color: "#334155"
    clip: true
    
    // ═══════════════════════════════════════════════════════════════
    // UI COMPONENTS
    // ═══════════════════════════════════════════════════════════════
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 24
        
        // Section 1: Chart / Trend (Left)
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12
            
            Text {
                text: selectedNode ? selectedNode.stageName.toUpperCase() + " // TREND" : "NO NODE SELECTED"
                font.pixelSize: 14
                font.weight: Font.Bold
                font.family: "JetBrains Mono"
                color: "#F8FAFC"
            }
            
            AXChart {
                Layout.fillWidth: true
                Layout.fillHeight: true
                type: "area"
                data: history.map(h => h.tp)
                showLegend: false
                showLabels: true
                primaryColor: "#3B82F6"
                backgroundColor: "transparent"
            }
        }
        
        // Section 2: Stats Table (Right)
        ColumnLayout {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            spacing: 8
            
            Text {
                text: "HISTORICAL SAMPLES"
                font.pixelSize: 12
                font.weight: Font.Bold
                font.family: "JetBrains Mono"
                color: "#94A3B8"
            }
            
            ListView {
                id: historyList
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: history
                clip: true
                delegate: RowLayout {
                    width: historyList.width
                    height: 25
                    
                    Text {
                        text: modelData.time
                        font.pixelSize: 11
                        font.family: "JetBrains Mono"
                        color: "#64748B"
                        Layout.preferredWidth: 60
                    }
                    Text {
                        text: "TP: " + modelData.tp.toFixed(0)
                        font.pixelSize: 11
                        font.family: "JetBrains Mono"
                        color: "#F8FAFC"
                        Layout.fillWidth: true
                    }
                    Text {
                        text: "LAT: " + modelData.lat.toFixed(2) + "ms"
                        font.pixelSize: 11
                        font.family: "JetBrains Mono"
                        color: modelData.lat > 5 ? "#EF4444" : "#22C55E"
                        font.weight: modelData.lat > 5 ? Font.Bold : Font.Normal
                    }
                }
            }
            
            // Diagnostic Tooltip
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
                color: "#0F172A"
                radius: 4
                Text {
                    anchors.centerIn: parent
                    text: selectedNode ? "DIAGNOSTIC: PID " + (Math.random()*10000).toFixed(0) : "IDLE"
                    font.pixelSize: 10
                    font.family: "JetBrains Mono"
                    color: "#3B82F6"
                }
            }
        }
    }
}
