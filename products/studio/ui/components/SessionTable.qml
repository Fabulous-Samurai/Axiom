import QtQuick
import QtQuick.Layouts
import "../theme"

Column {
    id: root
    width: parent.width

    property var sessionModel: []
    property var columnWidths: [100, 400, 100, 80, 80]

    function latencyColor(us) {
        if (us < 1000) return Colors.perfFast;
        if (us < 10000) return Colors.perfOk;
        if (us < 100000) return Colors.perfSlow;
        return Colors.perfCritical;
    }

    function formatLatency(us) {
        if (us < 1000) return us + " µs";
        return (us / 1000).toFixed(2) + " ms";
    }

    Row {
        height: 56; spacing: 0
        Repeater {
            model: ["Mode", "Expression", "Latency", "Status", "Action"]
            Text {
                width: columnWidths[index]
                text: modelData
                color: Colors.textSecondary
                font.weight: Typography.semibold
                font.pixelSize: Typography.small
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    ListView {
        width: parent.width
        height: 300
        model: sessionModel
        clip: true
        delegate: Rectangle {
            width: root.width
            height: 48
            color: hovered ? Colors.hover : (index % 2 === 0 ? "transparent" : Qt.rgba(0,0,0,0.02))

            property bool hovered: mouseArea.containsMouse

            Row {
                anchors.fill: parent
                spacing: 0

                Item {
                    width: columnWidths[0]; height: parent.height
                    ModeBadge { mode: modelData.mode; anchors.verticalCenter: parent.verticalCenter }
                }

                Item {
                    width: columnWidths[1]; height: parent.height
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.expression
                        font.family: Typography.code
                        font.pixelSize: Typography.bodyDefault
                        color: Colors.textPrimary
                        elide: Text.ElideRight
                        width: parent.width - Spacing.medium
                    }
                }

                Item {
                    width: columnWidths[2]; height: parent.height
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: formatLatency(modelData.latency_us)
                        color: latencyColor(modelData.latency_us)
                        font.family: Typography.code
                    }
                }

                Item {
                    width: columnWidths[3]; height: parent.height
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.success ? "✓" : "✗"
                        color: modelData.success ? Colors.success : Colors.error
                        font.pixelSize: Typography.h4
                    }
                }

                Item {
                    width: columnWidths[4]; height: parent.height
                    ActionButton {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "Run"
                        variant: "ghost"
                    }
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }
}