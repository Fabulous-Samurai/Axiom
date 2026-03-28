import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import Axiom.Expressway 1.0
import Axiom.UI 1.0
import "./components"
import "./theme"

Window {
    id: studioRoot
    visible: true
    width: 1400
    height: 900
    color: "#050505"
    title: "AXIOM Studio | Suite Evolution"

    readonly property color clrIndigo: "#0F172A"
    readonly property color clrCyan: "#7DD3FC"
    readonly property color clrGlass: Qt.rgba(15/255, 23/255, 42/255, 0.8)
    readonly property color clrBorder: Qt.rgba(125/255, 211/255, 252/255, 0.1)

    property string currentPage: "dashboard"
    property bool isSidebarExpanded: false

    // --- Telemetry Mock Store ---
    QtObject {
        id: telemetry
        property real throughput: 2540000
        property real latency: 0.05
        property real arenaUsage: 42.5
        property real jitCompiles: 12
        
        property Timer updater: Timer {
            interval: 1000; running: true; repeat: true
            onTriggered: {
                telemetry.throughput += (Math.random() - 0.5) * 50000
                telemetry.latency = 0.04 + Math.random() * 0.02
                telemetry.arenaUsage = 40 + Math.random() * 5
                if (Math.random() > 0.8) telemetry.jitCompiles += 1
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Sidebar
        Rectangle {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: isSidebarExpanded ? 220 : 80
            color: clrIndigo
            border.color: clrBorder

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: 20
                spacing: 10

                // Menu Toggle
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 56
                    color: "transparent"
                    
                    Image {
                        anchors.left: parent.left
                        anchors.leftMargin: isSidebarExpanded ? 24 : 28
                        anchors.verticalCenter: parent.verticalCenter
                        width: 24; height: 24
                        source: "file:///assets/icons/menu.svg"
                        fillMode: Image.PreserveAspectFit
                        opacity: 0.8
                        Behavior on anchors.leftMargin { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: isSidebarExpanded = !isSidebarExpanded
                    }
                }

                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: clrBorder; Layout.margins: 10 }

                SidebarItem { 
                    iconSource: "assets/icons/dashboard.svg"; text: "DASHBOARD"
                    active: currentPage === "dashboard"
                    isExpanded: isSidebarExpanded
                    onClicked: currentPage = "dashboard"
                }
                SidebarItem { 
                    iconSource: "assets/icons/pluto.svg"; text: "PLUTO SWARM"
                    active: currentPage === "pluto"
                    isExpanded: isSidebarExpanded
                    onClicked: currentPage = "pluto"
                }
                SidebarItem { 
                    iconSource: "assets/icons/zenith.svg"; text: "ZENITH JIT"
                    active: currentPage === "zenith"
                    isExpanded: isSidebarExpanded
                    onClicked: currentPage = "zenith"
                }

                Item { Layout.fillHeight: true }

                SidebarItem { 
                    iconSource: "assets/icons/settings.svg"; text: "SETTINGS"
                    active: currentPage === "settings"
                    isExpanded: isSidebarExpanded
                    onClicked: currentPage = "settings"
                }
            }
        }

        // Main Area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Top Status Bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                color: clrIndigo
                border.color: clrBorder

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 30
                    anchors.rightMargin: 30

                    Text {
                        text: "AXIOM ENGINE / " + currentPage.toUpperCase()
                        color: clrCyan
                        font.bold: true
                        font.family: "JetBrains Mono"
                    }

                    Item { Layout.fillWidth: true }

                    TelemetryBadge { label: "THR"; badgeValue: (telemetry.throughput / 1000000).toFixed(2) + "M"; accentColor: clrCyan }
                }
            }

            // Main Stack
            StackView {
                id: mainStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                initialItem: dashboardPage
            }
        }
    }

    // --- Page: Dashboard ---
    Component {
        id: dashboardPage
        Item {
            ScrollView {
                anchors.fill: parent
                contentWidth: parent.width
                clip: true

                ColumnLayout {
                    width: studioRoot.width - (isSidebarExpanded ? 220 : 80)
                    anchors.margins: 40
                    spacing: 30

                    RowLayout {
                        Layout.fillWidth: true
                        Column {
                            Text { text: "System Overview"; color: "white"; font.pixelSize: 32; font.bold: true }
                            Text { text: "Live High-Performance Compute Metrics"; color: "#94A3B8"; font.pixelSize: 14 }
                        }
                        Item { Layout.fillWidth: true }
                        StatusIndicator { label: "ENGINE"; status: "ACTIVE" }
                    }

                    // KPI Grid
                    GridLayout {
                        Layout.fillWidth: true
                        columns: width > 1100 ? 4 : (width > 600 ? 2 : 1)
                        columnSpacing: 20
                        rowSpacing: 20

                        KPICard {
                            title: "THROUGHPUT"
                            value: telemetry.throughput
                            unit: "ops/sec"
                            trend: 2.4
                            accentColor: Colors.info
                        }
                        KPICard {
                            title: "IPC LATENCY"
                            value: telemetry.latency
                            unit: "ms"
                            trend: -0.5
                            accentColor: Colors.success
                        }
                        KPICard {
                            title: "ARENA USAGE"
                            value: telemetry.arenaUsage
                            unit: "%"
                            trend: 0.1
                            accentColor: Colors.warning
                        }
                        KPICard {
                            title: "JIT NODES"
                            value: telemetry.jitCompiles
                            unit: "active"
                            trend: 0
                            accentColor: Colors.primary
                        }
                    }

                    // Main Waveform
                    GlassPanel {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 450
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 25
                            
                            RowLayout {
                                Text { text: "Throughput Waveform"; color: "white"; font.pixelSize: 18; font.bold: true }
                                Item { Layout.fillWidth: true }
                                ActionButton { text: "EXPORT LOGS"; variant: "ghost" }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                color: "#050505"
                                radius: 8
                                border.color: clrBorder
                                clip: true

                                ThroughputGraph {
                                    anchors.fill: parent
                                    anchors.margins: 20
                                    color: clrCyan
                                    maxPoints: 1000
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 20
                        
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            Text { anchors.centerIn: parent; text: "MEMORY FRAGMENTATION ANALYSIS"; color: clrCyan; opacity: 0.2; font.letterSpacing: 2 }
                        }
                        
                        GlassPanel {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            Text { anchors.centerIn: parent; text: "WORK-STEALING HEATMAP"; color: clrCyan; opacity: 0.2; font.letterSpacing: 2 }
                        }
                    }
                }
            }
        }
    }

    // --- Page: Generic ---
    Component {
        id: genericPage
        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 40
                Text { text: "Module Under Development"; color: "white"; font.pixelSize: 32 }
                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    color: clrGlass; border.color: clrBorder; radius: 12
                    Text { anchors.centerIn: parent; text: currentPage.toUpperCase(); color: clrCyan; opacity: 0.2 }
                }
            }
        }
    }

    // --- Mock Managers ---
    QtObject {
        id: zenithManager
        property string disassembly: ""
        function compileExpression(expr) {
            console.log("Compiling: " + expr)
            disassembly = "; Zenith JIT Output for: " + expr + "\n" +
                          "movsd xmm0, [rdi]\n" +
                          "mulsd xmm0, xmm0\n" +
                          "ret"
        }
    }

    // --- Page: Pluto ---
    Component {
        id: plutoPage
        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 40
                spacing: 20

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Pluto Swarm Navigator"; color: "white"; font.pixelSize: 32; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text { 
                        text: "Zoom: " + navigator.zoom.toFixed(2) + "x | Pan: " + Math.round(navigator.pan.x) + "," + Math.round(navigator.pan.y)
                        color: clrCyan
                        font.family: "JetBrains Mono"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000000"
                    border.color: clrBorder
                    radius: 12
                    clip: true

                    PlutoNavigator {
                        id: navigator
                        anchors.fill: parent
                        nodeColor: clrCyan
                        
                        WheelHandler {
                            onWheel: (event)=> {
                                var delta = event.angleDelta.y / 1200.0
                                navigator.zoom = Math.max(0.1, Math.min(10.0, navigator.zoom + delta))
                            }
                        }

                        DragHandler {
                            target: null
                            onTranslationChanged: {
                                navigator.pan = Qt.point(navigator.pan.x + translation.x / 10, 
                                                         navigator.pan.y + translation.y / 10)
                            }
                        }
                    }

                    // Instructions Overlay
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.margins: 20
                        width: 250; height: 80
                        color: Qt.rgba(0,0,0,0.6)
                        radius: 8
                        border.color: clrBorder
                        Column {
                            anchors.centerIn: parent
                            spacing: 5
                            Text { text: "🖱️ Wheel: Zoom In/Out"; color: "white"; font.pixelSize: 12 }
                            Text { text: "🖱️ Left Drag: Pan Swarm"; color: "white"; font.pixelSize: 12 }
                            Text { text: "🖱️ Left Click: Select Node"; color: "white"; font.pixelSize: 12 }
                        }
                    }
                }
            }
        }
    }

    onCurrentPageChanged: {
        if (currentPage === "dashboard") mainStack.replace(dashboardPage)
        else if (currentPage === "pluto") mainStack.replace(plutoPage)
        else if (currentPage === "zenith") mainStack.replace(zenithPage)
        else mainStack.replace(genericPage)
    }

    // --- Page: Zenith ---
    Component {
        id: zenithPage
        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 40
                spacing: 20

                Text { text: "Zenith JIT Disassembler"; color: "white"; font.pixelSize: 32; font.bold: true }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 15
                    
                    TextField {
                        id: exprInput
                        Layout.fillWidth: true
                        placeholderText: "Enter algebraic expression (e.g. x * x + y * 2)"
                        text: "x * x + y * 2 + z"
                        color: "white"
                        background: Rectangle { color: "#1a1a1a"; border.color: clrBorder; radius: 6 }
                        font.family: "JetBrains Mono"
                        onAccepted: jitExecutionManager.compileExpression(text)
                    }
                    
                    Button {
                        text: "COMPILE"
                        onClicked: jitExecutionManager.compileExpression(exprInput.text)
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#000000"
                    border.color: clrBorder
                    radius: 12
                    clip: true

                    ScrollView {
                        anchors.fill: parent
                        TextArea {
                            text: zenithManager.disassembly || "No code generated yet. Enter an expression above."
                            readOnly: true
                            color: clrCyan
                            font.family: "JetBrains Mono"
                            font.pixelSize: 14
                            background: null
                        }
                    }
                }
            }
        }
    }
}
