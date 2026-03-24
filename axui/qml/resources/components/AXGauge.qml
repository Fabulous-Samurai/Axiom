import QtQuick 2.15
import QtQuick.Layouts 1.15

/*
 * AXGauge - Circular progress/gauge indicator
 * 
 * Usage in .axui:
 * {
 *   "component": "Gauge",
 *   "props": {
 *     "value": "@engine.metrics.cpuUsage",
 *     "max": 100,
 *     "unit": "%",
 *     "thresholds": [
 *       { "value": 70, "color": "#22C55E" },
 *       { "value": 90, "color": "#EAB308" },
 *       { "value": 100, "color": "#EF4444" }
 *     ]
 *   }
 * }
 */

Item {
    id: root
    
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════
    
    property real value: 0
    property real min: 0
    property real max: 100
    property string unit: "%"
    property string label: ""
    property bool showValue: true
    property bool animated: true
    property int animationDuration: 800
    
    // Thresholds for color changes
    property var thresholds: [
        { value: 60, color: "#22C55E" },   // Green
        { value: 80, color: "#EAB308" },   // Yellow
        { value: 100, color: "#EF4444" }   // Red
    ]
    
    // Visual
    property real strokeWidth: 12
    property real startAngle: -225  // degrees
    property real endAngle: 45      // degrees
    property color backgroundColor: "#334155"
    property color textColor: "#F8FAFC"
    property color labelColor: "#94A3B8"
    
    // Glow effect
    property bool glowEnabled: true
    property real glowRadius: 8
    
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL
    // ═══════════════════════════════════════════════════════════════
    
    implicitWidth: 160
    implicitHeight: 160
    
    property real animatedValue: min
    
    Behavior on animatedValue {
        NumberAnimation {
            duration: animated ? animationDuration : 0
            easing.type: Easing.OutCubic
        }
    }
    
    onValueChanged: {
        animatedValue = Math.max(min, Math.min(max, value))
    }
    
    readonly property real normalizedValue: (animatedValue - min) / (max - min)
    readonly property real sweepAngle: (endAngle - startAngle) * normalizedValue
    
    readonly property color currentColor: {
        for (let i = thresholds.length - 1; i >= 0; i--) {
            if (normalizedValue * 100 <= thresholds[i].value) {
                return thresholds[i].color
            }
        }
        return thresholds[0].color
    }
    
    // ═══════════════════════════════════════════════════════════════
    // CANVAS
    // ═══════════════════════════════════════════════════════════════
    
    Canvas {
        id: canvas
        anchors.fill: parent
        
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            const centerX = width / 2
            const centerY = height / 2
            const radius = Math.min(width, height) / 2 - strokeWidth
            
            const startRad = startAngle * Math.PI / 180
            const endRad = endAngle * Math.PI / 180
            const valueRad = startRad + (sweepAngle * Math.PI / 180)
            
            // Background arc
            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, startRad, endRad)
            ctx.strokeStyle = backgroundColor
            ctx.lineWidth = strokeWidth
            ctx.lineCap = "round"
            ctx.stroke()
            
            // Value arc
            if (normalizedValue > 0) {
                ctx.beginPath()
                ctx.arc(centerX, centerY, radius, startRad, valueRad)
                ctx.strokeStyle = currentColor
                ctx.lineWidth = strokeWidth
                ctx.lineCap = "round"
                ctx.stroke()
            }
            
            // Tick marks
            const tickCount = 10
            for (let i = 0; i <= tickCount; i++) {
                const tickAngle = startRad + ((endRad - startRad) / tickCount) * i
                const tickInner = radius - strokeWidth / 2 - 5
                const tickOuter = radius - strokeWidth / 2 - 12
                
                ctx.beginPath()
                ctx.moveTo(
                    centerX + Math.cos(tickAngle) * tickInner,
                    centerY + Math.sin(tickAngle) * tickInner
                )
                ctx.lineTo(
                    centerX + Math.cos(tickAngle) * tickOuter,
                    centerY + Math.sin(tickAngle) * tickOuter
                )
                ctx.strokeStyle = i % 5 === 0 ? labelColor : backgroundColor
                ctx.lineWidth = i % 5 === 0 ? 2 : 1
                ctx.stroke()
            }
        }
        
        Connections {
            target: root
            function onAnimatedValueChanged() { canvas.requestPaint() }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // GLOW EFFECT
    // ═══════════════════════════════════════════════════════════════
    
    Canvas {
        visible: glowEnabled && normalizedValue > 0
        anchors.fill: parent
        opacity: 0.5
        
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            const centerX = width / 2
            const centerY = height / 2
            const radius = Math.min(width, height) / 2 - strokeWidth
            
            const startRad = startAngle * Math.PI / 180
            const valueRad = startRad + (sweepAngle * Math.PI / 180)
            
            ctx.shadowBlur = glowRadius
            ctx.shadowColor = currentColor
            
            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, startRad, valueRad)
            ctx.strokeStyle = currentColor
            ctx.lineWidth = strokeWidth
            ctx.lineCap = "round"
            ctx.stroke()
        }
        
        Connections {
            target: root
            function onAnimatedValueChanged() { requestPaint() }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // CENTER TEXT
    // ═══════════════════════════════════════════════════════════════
    
    Column {
        anchors.centerIn: parent
        spacing: 4
        
        Text {
            visible: showValue
            anchors.horizontalCenter: parent.horizontalCenter
            text: Math.round(animatedValue) + unit
            font.pixelSize: root.width * 0.2
            font.weight: Font.Bold
            color: textColor
        }
        
        Text {
            visible: label !== ""
            anchors.horizontalCenter: parent.horizontalCenter
            text: label
            font.pixelSize: root.width * 0.08
            color: labelColor
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // LIFECYCLE
    // ═══════════════════════════════════════════════════════════════
    
    Component.onCompleted: {
        animatedValue = min
        animatedValue = value
    }
}
