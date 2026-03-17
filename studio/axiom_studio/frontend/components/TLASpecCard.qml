import QtQuick
import "../theme"

GlassPanel {
    property string specName: ""
    property int statesExplored: 0
    property int searchDepth: 0
    property int invariantsVerified: 0
    property int livenessVerified: 0
    property string status: "verified"

    width: parent.width; height: 120

    Row {
        anchors.fill: parent; anchors.margins: Spacing.mediumLarge
        spacing: Spacing.xlarge

        Column {
            width: 300
            spacing: Spacing.small
            Text {
                text: specName
                font.pixelSize: Typography.h3
                font.weight: Typography.bold
                color: Colors.textPrimary
            }
            Row {
                spacing: Spacing.small
                Rectangle {
                    width: 12; height: 12; radius: 6
                    color: status === "verified" ? Colors.verified : (status === "violation" ? Colors.violation : Colors.unverified)
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: status.toUpperCase()
                    color: status === "verified" ? Colors.verified : (status === "violation" ? Colors.violation : Colors.unverified)
                    font.pixelSize: Typography.small
                    font.weight: Typography.bold
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        Column {
            Text { text: "States"; color: Colors.textSecondary; font.pixelSize: Typography.small }
            Text { text: statesExplored.toLocaleString(Qt.locale(), 'f', 0); color: Colors.textPrimary; font.pixelSize: Typography.h3; font.family: Typography.code }
        }

        Column {
            Text { text: "Depth"; color: Colors.textSecondary; font.pixelSize: Typography.small }
            Text { text: searchDepth; color: Colors.textPrimary; font.pixelSize: Typography.h3; font.family: Typography.code }
        }

        Column {
            Text { text: "Invariants"; color: Colors.textSecondary; font.pixelSize: Typography.small }
            Text { text: invariantsVerified; color: Colors.textPrimary; font.pixelSize: Typography.h3; font.family: Typography.code }
        }
        
        Column {
            visible: livenessVerified > 0
            Text { text: "Liveness"; color: Colors.textSecondary; font.pixelSize: Typography.small }
            Text { text: livenessVerified; color: Colors.textPrimary; font.pixelSize: Typography.h3; font.family: Typography.code }
        }
    }
}