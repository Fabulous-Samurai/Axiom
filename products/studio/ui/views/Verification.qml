import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../theme"
import "../components"

Item {
    id: root
    anchors.fill: parent

    property var specs: [
        { name: "AxiomWorkspaceScalability", states: 2187, depth: 18, invariants: 8, liveness: 1 },
        { name: "AxiomIpcProtocol", states: 14523, depth: 28, invariants: 9, liveness: 4 },
        { name: "AxiomDaemonQueueFairness", states: 8341, depth: 22, invariants: 0, liveness: 0 }
    ]

    ScrollView {
        anchors.fill: parent
        anchors.margins: Spacing.xlarge
        clip: true

        Column {
            width: parent.width
            spacing: Spacing.large

            Text {
                text: "TLA+ Formal Verification"
                font.family: Typography.heading
                font.pixelSize: Typography.h2
                color: Colors.textPrimary
                font.weight: Typography.semibold
            }

            Repeater {
                model: root.specs
                TLASpecCard {
                    specName: modelData.name
                    statesExplored: modelData.states
                    searchDepth: modelData.depth
                    invariantsVerified: modelData.invariants
                    livenessVerified: modelData.liveness
                    status: "verified"
                }
            }
        }
    }
}
