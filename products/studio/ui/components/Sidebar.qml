import QtQuick
import QtQuick.Layouts
import "../theme"

Rectangle {
    id: sidebar
    property bool collapsed: false
    signal pageChanged(string page)

    width: collapsed ? Spacing.sidebarCollapsed : Spacing.sidebarExpanded
    color: Colors.canvas

    Behavior on width {
        NumberAnimation { duration: 300; easing.type: Easing.InOutCubic }
    }

    Column {
        anchors.fill: parent
        spacing: Spacing.micro

        SidebarItem { icon: "⚡"; text: "Dashboard"; page: "dashboard"; active: true; onClicked: sidebar.pageChanged(page) }

        SidebarItem {
            icon: "📊"; text: "Modes"; page: "modes"; expandable: true; onClicked: sidebar.pageChanged(page)
        }
        Column {
            width: parent.width
            visible: !sidebar.collapsed
            SubItem { badge: Colors.modeAlgebraic; text: "Algebraic" }
            SubItem { badge: Colors.modeLinear; text: "Linear" }
            SubItem { badge: Colors.modeStatistics; text: "Statistics" }
            SubItem { badge: Colors.modeSymbolic; text: "Symbolic" }
            SubItem { badge: Colors.modeUnits; text: "Units" }
            SubItem { badge: Colors.modePlot; text: "Plot" }
        }

        SidebarItem { icon: "🎨"; text: "Graphics"; page: "graphics"; onClicked: sidebar.pageChanged(page) }
        SidebarItem { icon: "🔬"; text: "Benchmark"; page: "benchmark"; onClicked: sidebar.pageChanged(page) }
        SidebarItem { icon: "✓"; text: "Verification"; page: "verification"; onClicked: sidebar.pageChanged(page) }
        SidebarItem { icon: "⚙️"; text: "Config"; page: "configuration"; onClicked: sidebar.pageChanged(page) }
        SidebarItem { icon: "📝"; text: "Logs"; page: "logs"; onClicked: sidebar.pageChanged(page) }

        Rectangle { width: parent.width - 32; height: 1; color: Colors.border; anchors.horizontalCenter: parent.horizontalCenter }

        SidebarItem { icon: "💾"; text: "Workspace"; page: "workspace"; onClicked: sidebar.pageChanged(page) }
        SidebarItem { icon: "🔧"; text: "Terminal"; page: "terminal"; onClicked: sidebar.pageChanged(page) }
    }
}
