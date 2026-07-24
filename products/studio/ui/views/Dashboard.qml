import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../theme"
import "../components"

Item {
    id: root
    width: parent ? parent.width : 800
    height: parent ? parent.height : 600

    property var sessionHistory: [
        { mode: "algebraic", expression: "2 + 2", latency_us: 15, success: true },
        { mode: "symbolic", expression: "diff(sin(x)*x, x)", latency_us: 450, success: true },
        { mode: "linear", expression: "invert([[1,2],[3,4]])", latency_us: 1200, success: true }
    ]

    Connections {
        target: engineBridge
        function onCalculationCompleted(result) {
            var arr = sessionHistory;
            arr.unshift(result);
            if(arr.length > 50) arr.pop();
            sessionHistory = arr;
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Spacing.xlarge
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: Spacing.xlarge

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Dashboard / Real-time Metrics"
                    font.family: Typography.heading
                    font.pixelSize: Typography.h2
                    color: Colors.textPrimary
                    font.weight: Typography.semibold
                }
                Item { Layout.fillWidth: true }
                ActionButton { text: "+ New"; variant: "primary" }
                ActionButton { text: "⟳"; variant: "ghost" }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: width > Spacing.breakpointDesktop ? 4 : (width > Spacing.breakpointMobile ? 2 : 1)
                columnSpacing: Spacing.large
                rowSpacing: Spacing.large

                KPICard { title: "Throughput"; value: engineBridge.throughput; unit: "ops/s"; Layout.fillWidth: true }
                KPICard { title: "Memory"; value: engineBridge.memoryMb; unit: "MB"; Layout.fillWidth: true }
                KPICard { title: "Active Mode"; value: 1; unit: "algebraic"; Layout.fillWidth: true }
                KPICard { title: "Workers"; value: 4; unit: "threads"; Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Spacing.large

                GlassPanel {
                    Layout.fillWidth: true
                    height: 200
                    Text { anchors.centerIn: parent; text: "Latency Chart (real-time)"; color: Colors.textSecondary }
                }
                GlassPanel {
                    Layout.fillWidth: true
                    height: 200
                    Text { anchors.centerIn: parent; text: "Mode Distribution"; color: Colors.textSecondary }
                }
            }

            GlassPanel {
                Layout.fillWidth: true
                height: 400
                Column {
                    anchors.fill: parent
                    anchors.margins: Spacing.mediumLarge
                    spacing: Spacing.medium
                    Text {
                        text: "Recent Calculations"
                        font.pixelSize: Typography.h3
                        font.weight: Typography.semibold
                        color: Colors.textPrimary
                    }
                    SessionTable {
                        id: sessionTable
                        width: parent.width
                        sessionModel: root.sessionHistory
                    }
                }
            }
        }
    }
}
