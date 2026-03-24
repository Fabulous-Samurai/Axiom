import QtQuick 2.15
import QtQuick.Layouts 1.15

GridLayout {
    id: root
    property int columns: 2
    property real spacing: 8
    columns: root.columns
    rowSpacing: root.spacing
    columnSpacing: root.spacing
}
