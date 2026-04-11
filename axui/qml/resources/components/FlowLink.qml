import QtQuick 2.15

Item {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════
    property point startPoint: Qt.point(0, 0)
    property point endPoint: Qt.point(100, 100)
    property real throughput: 0
    property real maxThroughput: 1000000

    // Heat Visualization
    readonly property real heatFactor: Math.min(1.0, throughput / maxThroughput)
    readonly property real lineWidth: 1 + (heatFactor * 8)
    readonly property color lineColor: {
        if (heatFactor > 0.8) return "#3B82F6" // Standard Blue, but thick
        return "#475569" // Muted Slate
    }

    // ═══════════════════════════════════════════════════════════════
    // UI COMPONENTS
    // ═══════════════════════════════════════════════════════════════

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            ctx.beginPath();
            ctx.moveTo(startPoint.x, startPoint.y);

            // Cubic bezier for smooth flow
            var cp1x = startPoint.x + (endPoint.x - startPoint.x) / 2;
            var cp1y = startPoint.y;
            var cp2x = startPoint.x + (endPoint.x - startPoint.x) / 2;
            var cp2y = endPoint.y;

            ctx.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, endPoint.x, endPoint.y);

            // Shadow / Glow for high heat
            if (heatFactor > 0.5) {
                ctx.shadowBlur = 10;
                ctx.shadowColor = Qt.rgba(59/255, 130/255, 246/255, heatFactor * 0.5);
            }

            ctx.strokeStyle = lineColor;
            ctx.lineWidth = lineWidth;
            ctx.lineCap = "round";
            ctx.stroke();

            // Flow Particles (Optional animation)
            if (throughput > 0) {
                drawParticle(ctx, cp1x, cp1y, cp2x, cp2y);
            }
        }

        property real particlePos: 0
        NumberAnimation on particlePos {
            from: 0; to: 1; duration: 2000; loops: Animation.Infinite
            running: throughput > 0
        }

        function drawParticle(ctx, cp1x, cp1y, cp2x, cp2y) {
            // Very simple particle at current particlePos along the curve
            // For simplicity, we use linear interpolation for the example,
            // but a true bezier interp would be better.
            var t = particlePos;
            var cx = (1-t)*(1-t)*(1-t)*startPoint.x + 3*(1-t)*(1-t)*t*cp1x + 3*(1-t)*t*t*cp2x + t*t*t*endPoint.x;
            var cy = (1-t)*(1-t)*(1-t)*startPoint.y + 3*(1-t)*(1-t)*t*cp1y + 3*(1-t)*t*t*cp2y + t*t*t*endPoint.y;

            ctx.beginPath();
            ctx.arc(cx, cy, 2 + (heatFactor * 2), 0, Math.PI * 2);
            ctx.fillStyle = "#F8FAFC";
            ctx.shadowBlur = 0; // Reset shadow for particle
            ctx.fill();
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Connections {
            target: root
            function onThroughputChanged() { canvas.requestPaint() }
            function onParticlePosChanged() { canvas.requestPaint() }
        }
    }
}
