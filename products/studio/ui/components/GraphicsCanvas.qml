import QtQuick
import "../theme"

Rectangle {
    id: canvasRoot
    property string source: ""
    property real zoom: 1.0
    property real panX: 0.0
    property real panY: 0.0
    property string expression: "" // Bind this to the current expression

    signal navigationChanged()

    onZoomChanged: refineTimer.restart()
    onPanXChanged: refineTimer.restart()
    onPanYChanged: refineTimer.restart()

    Timer {
        id: refineTimer
        interval: 300 // 300ms debounce
        onTriggered: {
            if (expression !== "") {
                // Update engine with current viewport for Adaptive Resolution
                engineBridge.evaluateGraphics(expression, 512, zoom, panX, panY)
            }
        }
    }
    color: Colors.canvas
    border.color: Colors.border
    border.width: 1
    radius: Spacing.radiusMedium
    clip: true

    ShaderEffect {
        id: effect
        anchors.fill: parent
        anchors.margins: 1

        property variant sourceTexture: ShaderEffectSource {
            sourceItem: img
            hideSource: true
        }
        property real zoomFactor: canvasRoot.zoom
        property vector2d offset: Qt.vector2d(canvasRoot.panX, canvasRoot.panY)

        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform lowp float qt_Opacity;
            uniform sampler2D sourceTexture;
            uniform highp float zoomFactor;
            uniform highp vec2 offset;

            void main() {
                highp vec2 uv = (qt_TexCoord0 - 0.5) / zoomFactor + 0.5 - offset;
                lowp vec4 tex = texture2D(sourceTexture, uv);
                gl_FragColor = tex * qt_Opacity;
            }
        "
    }

    Image {
        id: img
        source: canvasRoot.source
        visible: false
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: false
    }

    MouseArea {
        anchors.fill: parent
        drag.target: null
        scrollGestureEnabled: true

        property real lastX: 0
        property real lastY: 0

        onPressed: (mouse) => {
            lastX = mouse.x
            lastY = mouse.y
        }

        onPositionChanged: (mouse) => {
            if (pressed) {
                let dx = (mouse.x - lastX) / width
                let dy = (mouse.y - lastY) / height
                canvasRoot.panX += dx / canvasRoot.zoom
                canvasRoot.panY += dy / canvasRoot.zoom
                lastX = mouse.x
                lastY = mouse.y
            }
        }

        onWheel: (wheel) => {
            let zoomDelta = wheel.angleDelta.y > 0 ? 1.1 : 0.9
            let newZoom = canvasRoot.zoom * zoomDelta
            if (newZoom >= 0.1 && newZoom <= 50.0) {
                canvasRoot.zoom = newZoom
            }
        }
    }

    Text {
        visible: img.status !== Image.Ready
        anchors.centerIn: parent
        text: "No Output"
        color: Colors.textTertiary
        font.pixelSize: Typography.bodyLarge
    }

    // Zoom Indicator
    Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Spacing.medium
        width: 60; height: 24; radius: 12
        color: "#AA000000"
        Text {
            anchors.centerIn: parent
            text: (canvasRoot.zoom * 100).toFixed(0) + "%"
            color: "white"
            font.pixelSize: 10
        }
    }
}
