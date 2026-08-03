import QtQuick
import Qt5Compat.GraphicalEffects
import "../theme"

Rectangle {
    id: root

    property int blurRadius: 12
    property real glassOpacity: 0.2
    property real borderOpacity: 0.1

    color: "transparent"
    radius: Spacing.radiusXL

    // Arka plan (Bulanık kısım)
    Rectangle {
        id: background
        anchors.fill: parent
        color: Colors.surface
        opacity: root.glassOpacity
        radius: root.radius
        border.color: Colors.glassBorder()
        border.width: 1

        layer.enabled: true
        layer.effect: FastBlur {
            radius: root.blurRadius
        }
    }

    // İçerik alanı (Bulanık olmayan kısım)
    Item {
        id: content
        anchors.fill: parent
    }
}
