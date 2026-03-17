import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import "theme"
import "components"
import "views"

Window {
    id: mainWindow
    visible: true
    width: 1280
    height: 800
    minimumWidth: 800
    minimumHeight: 600
    color: Colors.backdrop
    title: "AXIOM Studio v1.0.0 — Powered by AXIOM Engine v3.1.1"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            id: sidebar
            Layout.fillHeight: true
            onPageChanged: (page) => {
                if (page === "dashboard") contentStack.replace(dashboardView);
                else if (page === "benchmark") contentStack.replace(benchmarkView);
                else if (page === "verification") contentStack.replace(verificationView);
                else if (page === "graphics") contentStack.replace(graphicsView);
                else if (page === "modes") contentStack.replace(placeholderView, {title: "Modes & Settings"});
                else if (page === "configuration") contentStack.replace(placeholderView, {title: "Configuration"});
                else if (page === "logs") contentStack.replace(placeholderView, {title: "System Logs"});
                else if (page === "workspace") contentStack.replace(placeholderView, {title: "Workspace Manager"});
                else if (page === "terminal") contentStack.replace(placeholderView, {title: "Native Terminal"});
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Navbar {
                Layout.fillWidth: true
            }

            StackView {
                id: contentStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                initialItem: dashboardView

                clip: true // İçeriğin taşmasını engelle

                pushEnter: Transition {
                    PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
                }
                pushExit: Transition {
                    PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 200 }
                }
            }
        }
    }

    Component { id: dashboardView; Dashboard {} }
    Component { id: benchmarkView; Benchmark {} }
    Component { id: verificationView; Verification {} }
    Component { id: graphicsView; Graphics {} }
    Component { id: placeholderView; PlaceholderView {} }

    Connections {
        target: engineBridge
        function onErrorOccurred(error) {
            errorToast.show(error)
        }
    }

    Toast {
        id: errorToast
        anchors.top: parent.top
        anchors.topMargin: Spacing.xlarge
        // x-koordinatı Toast.qml içindeki slide animasyonları tarafından kontrol ediliyor
    }
}