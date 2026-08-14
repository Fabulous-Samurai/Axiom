import QtQuick
import "../theme"

Rectangle {
    id: root
    property string placeholder: "Enter expression..."
    property string mode: "algebraic"
    property alias text: input.text
    signal accepted()

    width: parent.width; height: 48
    radius: Spacing.radiusMedium
    color: Colors.surface
    border.color: input.activeFocus ? Colors.primary : Colors.border
    border.width: 1

    Behavior on border.color { ColorAnimation { duration: 150 } }

    TextInput {
        id: input
        anchors.fill: parent
        anchors.leftMargin: Spacing.medium
        anchors.rightMargin: Spacing.medium
        verticalAlignment: TextInput.AlignVCenter
        font.family: Typography.code
        font.pixelSize: Typography.bodyLarge
        color: Colors.textPrimary
        selectionColor: Colors.primary
        selectedTextColor: Colors.surface

        Text {
            visible: !input.text && !input.activeFocus
            text: root.placeholder // parent yerine root (Rectangle) kullanıyoruz
            color: Colors.textTertiary
            font.family: Typography.body
            font.pixelSize: Typography.bodyLarge
            anchors.verticalCenter: parent.verticalCenter
        }

        onAccepted: parent.accepted()
    }
}
