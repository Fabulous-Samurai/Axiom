import QtQuick
import Qt5Compat.GraphicalEffects
import "../theme"

Glow {
    id: root
    property bool active: false

    radius: active ? 12 : 0
    color: Colors.primary
    opacity: active ? 0.4 : 0

    Behavior on radius { NumberAnimation { duration: 200 } }
    Behavior on opacity { NumberAnimation { duration: 200 } }
}
