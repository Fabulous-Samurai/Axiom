import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

/*
 * AXDataGrid - Sortable, filterable data table
 *
 * Usage in .axui:
 * {
 *   "component": "DataGrid",
 *   "props": {
 *     "columns": [
 *       { "key": "name", "label": "Name", "width": 200 },
 *       { "key": "value", "label": "Value", "align": "right" }
 *     ],
 *     "data": "@engine.data.users",
 *     "sortable": true
 *   }
 * }
 */

Rectangle {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    // Required
    property var columns: []   // [{key, label, width?, align?, sortable?}]
    property var data: []      // Array of objects

    // Optional
    property bool sortable: true
    property bool filterable: false
    property bool selectable: false
    property bool striped: true
    property real rowHeight: 48
    property real headerHeight: 56

    // Theme
    property color backgroundColor: "#1E293B"
    property color headerBackground: "#0F172A"
    property color rowBackground: "#1E293B"
    property color rowAlternate: "#283548"
    property color rowHover: "#334155"
    property color borderColor: "#334155"
    property color textColor: "#F8FAFC"
    property color headerTextColor: "#94A3B8"
    property color selectedColor: "#3B82F6"

    // Glass
    property bool glassEnabled: false
    property real glassBlur: 8

    // ═══════════════════════════════════════════════════════════════
    // INTERNAL STATE
    // ═══════════════════════════════════════════════════════════════

    property string sortColumn: ""
    property bool sortAscending: true
    property var selectedRows: []
    property string filterText: ""

    implicitWidth: 600
    implicitHeight: headerHeight + (Math.min(filteredData.length, 10) * rowHeight)

    radius: 8
    color: backgroundColor
    border.color: borderColor
    border.width: 1
    clip: true

    // Filtered and sorted data
    readonly property var filteredData: {
        let result = [...data]

        // Filter
        if (filterText.length > 0) {
            const search = filterText.toLowerCase()
            result = result.filter(row => {
                return columns.some(col => {
                    const value = String(row[col.key] || "").toLowerCase()
                    return value.includes(search)
                })
            })
        }

        // Sort
        if (sortColumn !== "") {
            result.sort((a, b) => {
                const aVal = a[sortColumn]
                const bVal = b[sortColumn]

                if (typeof aVal === "number" && typeof bVal === "number") {
                    return sortAscending ? aVal - bVal : bVal - aVal
                }

                const aStr = String(aVal || "")
                const bStr = String(bVal || "")
                return sortAscending
                    ? aStr.localeCompare(bStr)
                    : bStr.localeCompare(aStr)
            })
        }

        return result
    }

    // ═══════════════════════════════════════════════════════════════
    // SIGNALS
    // ═══════════════════════════════════════════════════════════════

    signal rowClicked(int index, var rowData)
    signal rowDoubleClicked(int index, var rowData)
    signal selectionChanged(var selectedIndices)
    signal sortChanged(string column, bool ascending)

    // ═══════════════════════════════════════════════════════════════
    // LAYOUT
    // ═══════════════════════════════════════════════════════════════

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ─── Filter Row ──────────────────────────────────────────
        Rectangle {
            visible: root.filterable
            Layout.fillWidth: true
            height: 48
            color: headerBackground

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8

                TextField {
                    Layout.fillWidth: true
                    placeholderText: "Search..."
                    background: Rectangle {
                        color: rowBackground
                        radius: 4
                        border.color: borderColor
                    }
                    color: textColor

                    onTextChanged: root.filterText = text
                }
            }
        }

        // ─── Header ──────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: headerHeight
            color: headerBackground

            Row {
                anchors.fill: parent

                Repeater {
                    model: root.columns

                    Rectangle {
                        width: modelData.width || (root.width / root.columns.length)
                        height: headerHeight
                        color: "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 16
                            spacing: 8

                            Text {
                                Layout.fillWidth: true
                                text: modelData.label || modelData.key
                                font.pixelSize: 12
                                font.weight: Font.Medium
                                font.capitalization: Font.AllUppercase
                                color: headerTextColor
                                horizontalAlignment: {
                                    if (modelData.align === "right") return Text.AlignRight
                                    if (modelData.align === "center") return Text.AlignHCenter
                                    return Text.AlignLeft
                                }
                            }

                            // Sort indicator
                            Text {
                                visible: root.sortable && root.sortColumn === modelData.key
                                text: root.sortAscending ? "▲" : "▼"
                                font.pixelSize: 10
                                color: selectedColor
                            }
                        }

                        // Sort click handler
                        MouseArea {
                            anchors.fill: parent
                            enabled: root.sortable && (modelData.sortable !== false)
                            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                            onClicked: {
                                if (root.sortColumn === modelData.key) {
                                    root.sortAscending = !root.sortAscending
                                } else {
                                    root.sortColumn = modelData.key
                                    root.sortAscending = true
                                }
                                root.sortChanged(root.sortColumn, root.sortAscending)
                            }
                        }

                        // Column divider
                        Rectangle {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            width: 1
                            height: parent.height * 0.5
                            color: borderColor
                        }
                    }
                }
            }
        }

        // ─── Header/Body Divider ─────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: borderColor
        }

        // ─── Body ────────────────────────────────────────────────
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            model: root.filteredData

            delegate: Rectangle {
                width: listView.width
                height: root.rowHeight
                color: {
                    if (mouseArea.containsMouse) return rowHover
                    if (root.selectable && root.selectedRows.includes(index))
                        return Qt.rgba(selectedColor.r, selectedColor.g,
                                       selectedColor.b, 0.2)
                    if (root.striped && index % 2 === 1) return rowAlternate
                    return rowBackground
                }

                Behavior on color {
                    ColorAnimation { duration: 100 }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        if (root.selectable) {
                            const idx = root.selectedRows.indexOf(index)
                            if (idx >= 0) {
                                root.selectedRows.splice(idx, 1)
                            } else {
                                root.selectedRows.push(index)
                            }
                            root.selectedRows = [...root.selectedRows]
                            root.selectionChanged(root.selectedRows)
                        }
                        root.rowClicked(index, modelData)
                    }

                    onDoubleClicked: {
                        root.rowDoubleClicked(index, modelData)
                    }
                }

                Row {
                    anchors.fill: parent

                    Repeater {
                        model: root.columns

                        Item {
                            width: modelData.width || (root.width / root.columns.length)
                            height: root.rowHeight

                            Text {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: {
                                    if (modelData.align === "right") return Text.AlignRight
                                    if (modelData.align === "center") return Text.AlignHCenter
                                    return Text.AlignLeft
                                }

                                text: {
                                    const value = filteredData[index][modelData.key]
                                    if (modelData.format === "currency") {
                                        return "$" + Number(value).toLocaleString()
                                    }
                                    if (modelData.format === "percent") {
                                        return Number(value).toFixed(1) + "%"
                                    }
                                    return String(value || "")
                                }

                                font.pixelSize: 14
                                color: textColor
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                // Row divider
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: borderColor
                    opacity: 0.5
                }
            }

            // Empty state
            Text {
                anchors.centerIn: parent
                visible: root.filteredData.length === 0
                text: root.filterText.length > 0
                      ? "No results found"
                      : "No data"
                font.pixelSize: 14
                color: headerTextColor
            }

            ScrollBar.vertical: ScrollBar {
                active: true
                policy: ScrollBar.AsNeeded
            }
        }
    }
}
