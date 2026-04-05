import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../theme"
import "../components"

Item {
    id: root
    anchors.fill: parent

    property var checks: [
        { name: "Parser Throughput", budget: 2000000, measured: 2500000, unit: "ops/s", passed: true },
        { name: "Graphics Eval (128x128)", budget: 1.6, measured: 1.5, unit: "ms", passed: true },
        { name: "Lookup p99", budget: 5, measured: 2.5, unit: "µs", passed: true }
    ]

    property int passedCount: 3
    property int totalCount: 3
    property bool allPassed: passedCount === totalCount

    Connections {
        target: engineBridge
        function onBenchmarkCompleted(result) {
            var newChecks = [];
            var passed = 0;
            for (var i = 0; i < result.benchmarks.length; i++) {
                var b = result.benchmarks[i];
                var isPassed = b.ops_per_sec > 50000; // Örnek eşik
                newChecks.push({
                    name: b.name.replace(/_/g, " "),
                    budget: 50000,
                    measured: b.ops_per_sec,
                    unit: "ops/s",
                    passed: isPassed
                });
                if (isPassed) passed++;
            }
            root.checks = newChecks;
            root.passedCount = passed;
            root.totalCount = newChecks.length;
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: Spacing.xlarge
        clip: true

        Column {
            width: parent.width
            spacing: Spacing.large

            RowLayout {
                width: parent.width
                Text { text: "Benchmark Scorecard"; font.pixelSize: Typography.h2; color: Colors.textPrimary; font.family: Typography.heading }
                Item { Layout.fillWidth: true }
                ActionButton { text: "Run Full Suite"; onClicked: engineBridge.runBenchmark() }
            }

            Grid {
                columns: width > Spacing.breakpointDesktop ? 2 : 1
                spacing: Spacing.large
                width: parent.width

                Repeater {
                    model: root.checks
                    BenchmarkCard {
                        checkName: modelData.name
                        budget: modelData.budget
                        measured: modelData.measured
                        unit: modelData.unit
                        passed: modelData.passed
                    }
                }
            }

            GlassPanel {
                width: parent.width
                height: 80
                Row {
                    anchors.centerIn: parent
                    Text {
                        text: passedCount + " / " + totalCount + " PASSED"
                        font.pixelSize: Typography.h2
                        font.weight: Typography.bold
                        color: allPassed ? Colors.success : Colors.warning
                    }
                }
            }
        }
    }
}
