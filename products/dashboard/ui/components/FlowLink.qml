import QtQuick 2.15

/*
 * FlowLink — Animated data path between pipeline nodes
 *
 * Pulsing glow particles flow from source to target.
 * Speed/density correlates to throughput ratio.
 */

Item {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC
    // ═══════════════════════════════════════════════════════════════

    property point startPoint: Qt.point(0, 0)
    property point endPoint: Qt.point(100, 0)
    property real throughputRatio: 0.0      // 0.0 - 1.0
    property real dropRate: 0.0             // 0.0 - 1.0
    property bool active: false
    property color linkColor: "#3B82F6"
    property color errorColor: "#EF4444"
    property bool backpressure: false

    // ═══════════════════════════════════════════════════════════════
    // COMPUTED
    // ═══════════════════════════════════════════════════════════════

    readonly property real linkLength: {
        var dx = endPoint.x - startPoint.x
        var dy = endPoint.y - startPoint.y
        return Math.sqrt(dx * dx + dy * dy)
    }

    readonly property real linkAngle: {
        return Math.atan2(
            endPoint.y - startPoint.y,
            endPoint.x - startPoint.x
        ) * 180 / Math.PI
    }

    readonly property int particleCount: Math.max(1, Math.floor(throughputRatio * 6))
    readonly property int particleSpeed: {
        if (!active) return 0
        return Math.max(500, 3000 - (throughputRatio * 2500))
    }

    readonly property color currentColor: (dropRate > 0.1 || backpressure) ? errorColor : linkColor

    // ═══════════════════════════════════════════════════════════════
    // BASE LINE
    // ═══════════════════════════════════════════════════════════════

    Canvas {
        id: lineCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Base line (dim)
            ctx.beginPath()
            ctx.moveTo(startPoint.x, startPoint.y)
            ctx.lineTo(endPoint.x, endPoint.y)
            ctx.strokeStyle = Qt.rgba(
                currentColor.r, currentColor.g, currentColor.b,
                active ? 0.3 : 0.1
            )
            ctx.lineWidth = 2 + (throughputRatio * 5)
            ctx.setLineDash(active ? [] : [8, 8])
            ctx.stroke()

            // Direction arrow (ortada)
            if (active) {
                var midX = (startPoint.x + endPoint.x) / 2
                var midY = (startPoint.y + endPoint.y) / 2
                var angle = Math.atan2(
                    endPoint.y - startPoint.y,
                    endPoint.x - startPoint.x
                )
                var arrowSize = 8

                ctx.beginPath()
                ctx.moveTo(
                    midX + Math.cos(angle) * arrowSize,
                    midY + Math.sin(angle) * arrowSize
                )
                ctx.lineTo(
                    midX + Math.cos(angle + 2.5) * arrowSize,
                    midY + Math.sin(angle + 2.5) * arrowSize
                )
                ctx.lineTo(
                    midX + Math.cos(angle - 2.5) * arrowSize,
                    midY + Math.sin(angle - 2.5) * arrowSize
                )
                ctx.closePath()
                ctx.fillStyle = Qt.rgba(
                    currentColor.r, currentColor.g, currentColor.b, 0.6
                )
                ctx.fill()
            }
        }

        Connections {
            target: root
            function onActiveChanged() { lineCanvas.requestPaint() }
            function onStartPointChanged() { lineCanvas.requestPaint() }
            function onEndPointChanged() { lineCanvas.requestPaint() }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // FLOWING PARTICLES
    // ═══════════════════════════════════════════════════════════════

    Repeater {
        model: active ? particleCount : 0

        Rectangle {
            id: particle
            width: 6 + (throughputRatio * 4)
            height: width
            radius: width / 2
            color: currentColor
            opacity: 0

            // Glow effect (simplified for Qt 6 compatibility)
            border.color: Qt.rgba(currentColor.r, currentColor.g, currentColor.b, 0.5)
            border.width: backpressure ? 3 : 2

            // Jitter for backpressure
            NumberAnimation on x {
                running: backpressure
                loops: Animation.Infinite
                from: -2; to: 2; duration: 50
            }

            // Flow animation
            PathAnimation {
                id: flowAnimation
                running: active
                loops: Animation.Infinite
                duration: particleSpeed

                // Stagger start time per particle
                Component.onCompleted: {
                    if (active) {
                        flowAnimation.start()
                    }
                }

                path: Path {
                    startX: startPoint.x
                    startY: startPoint.y
                    PathLine {
                        x: endPoint.x
                        y: endPoint.y
                    }
                }
            }

            // Fade in/out along path
            SequentialAnimation on opacity {
                running: active
                loops: Animation.Infinite
                NumberAnimation { to: 0.9; duration: particleSpeed * 0.2 }
                NumberAnimation { to: 0.9; duration: particleSpeed * 0.6 }
                NumberAnimation { to: 0.0; duration: particleSpeed * 0.2 }
            }

            // Stagger each particle
            Component.onCompleted: {
                // Staggering would require PathInterpolator or individual timers
                // Removing problematic currentTime assignment
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // DROP RATE INDICATOR
    // ═══════════════════════════════════════════════════════════════

    Text {
        visible: active && dropRate > 0.05
        x: (startPoint.x + endPoint.x) / 2
        y: (startPoint.y + endPoint.y) / 2 - 16
        text: "▼ " + (dropRate * 100).toFixed(1) + "% drop"
        font.pixelSize: 9
        color: errorColor
        font.family: "JetBrains Mono"
    }
}
