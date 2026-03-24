import QtQuick 2.15
import QtQuick.Layouts 1.15

/*
 * AXChart - Line, Bar, Area charts
 * 
 * Usage in .axui:
 * {
 *   "component": "Chart",
 *   "props": {
 *     "type": "line",
 *     "data": "@engine.metrics.history",
 *     "showLegend": true
 *   }
 * }
 */

Rectangle {
    id: root
    
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════
    
    // Required
    property string type: "line"  // "line", "bar", "area", "donut"
    property var data: []         // [{label, value, color?}] or [[x, y], ...]
    
    // Series (for multi-line charts)
    property var series: []       // [{name, data: [], color}]
    
    // Options
    property bool showLegend: true
    property bool showGrid: true
    property bool showLabels: true
    property bool showTooltip: true
    property bool animated: true
    property int animationDuration: 800
    
    // Axis
    property string xAxisLabel: ""
    property string yAxisLabel: ""
    property var xAxisTicks: null  // Auto if null
    property var yAxisTicks: null
    
    // Thresholds (Phase 6.1 Fix)
    property real warningThreshold: 0
    property real criticalThreshold: 0
    property color warningColor: "#F59E0B"  // Orange
    property color criticalColor: "#EF4444" // Red
    property bool showThresholds: warningThreshold > 0 || criticalThreshold > 0
    
    // Theme
    property color backgroundColor: "transparent"
    property color gridColor: "#334155"
    property color textColor: "#94A3B8"
    property color primaryColor: "#3B82F6"
    property var colorPalette: [
        "#3B82F6", "#22C55E", "#EAB308", "#EF4444", 
        "#8B5CF6", "#EC4899", "#06B6D4", "#F97316"
    ]
    
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL STATE
    // ═══════════════════════════════════════════════════════════════
    
    property real animationProgress: 0
    property int hoveredIndex: -1
    property point mousePos: Qt.point(-1, -1)
    
    implicitWidth: 400
    implicitHeight: 300
    
    color: backgroundColor
    radius: 8
    
    // Animation
    NumberAnimation {
        id: entryAnimation
        target: root
        property: "animationProgress"
        from: 0
        to: 1
        duration: animated ? animationDuration : 0
        easing.type: Easing.OutCubic
    }
    
    Component.onCompleted: {
        if (animated) {
            entryAnimation.start()
        } else {
            animationProgress = 1
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // COMPUTED
    // ═══════════════════════════════════════════════════════════════
    
    readonly property var chartData: {
        // Normalize data format
        if (series.length > 0) {
            return series
        }
        
        // Single series from data
        return [{
            name: "Data",
            data: data,
            color: primaryColor
        }]
    }
    
    readonly property real maxValue: {
        let max = 0
        chartData.forEach(s => {
            s.data.forEach(d => {
                const val = typeof d === "object" ? (d.value || d.y || 0) : d
                if (val > max) max = val
            })
        })
        return Math.max(1, max * 1.1)  // 10% padding, floor of 1.0 to avoid NaN
    }
    
    readonly property real minValue: {
        let min = Infinity
        chartData.forEach(s => {
            s.data.forEach(d => {
                const val = typeof d === "object" ? (d.value || d.y || 0) : d
                if (val < min) min = val
            })
        })
        return min > 0 ? 0 : min
    }
    
    // ═══════════════════════════════════════════════════════════════
    // CHART CANVAS
    // ═══════════════════════════════════════════════════════════════
    
    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 16
        anchors.bottomMargin: showLegend ? 48 : 16
        
        renderTarget: Canvas.Image
        renderStrategy: Canvas.Threaded
        antialiasing: true

        onWidthChanged: canvas.requestPaint()
        onHeightChanged: canvas.requestPaint()
        
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            
            const padding = { top: 20, right: 20, bottom: 30, left: 50 }
            const chartWidth = width - padding.left - padding.right
            const chartHeight = height - padding.top - padding.bottom
            
            // Draw grid
            if (showGrid) {
                drawGrid(ctx, padding, chartWidth, chartHeight)
            }
            
            // Draw chart based on type
            switch (type) {
                case "line":
                    drawLineChart(ctx, padding, chartWidth, chartHeight)
                    break
                case "bar":
                    drawBarChart(ctx, padding, chartWidth, chartHeight)
                    break
                case "area":
                    drawAreaChart(ctx, padding, chartWidth, chartHeight)
                    break
                case "donut":
                    drawDonutChart(ctx, padding)
                    break
            }

            // Draw Threshold Lines (Phase 6.1 Fix)
            if (showThresholds && type !== "donut") {
                drawThresholdLine(ctx, padding, chartWidth, chartHeight, criticalThreshold, criticalColor, "CRITICAL")
                drawThresholdLine(ctx, padding, chartWidth, chartHeight, warningThreshold, warningColor, "WARNING")
            }
            
            // Draw axes labels
            if (showLabels) {
                drawAxisLabels(ctx, padding, chartWidth, chartHeight)
            }
        }
        
        function drawGrid(ctx, padding, chartWidth, chartHeight) {
            ctx.strokeStyle = gridColor
            ctx.lineWidth = 0.5
            ctx.setLineDash([4, 4])
            
            // Horizontal grid lines (5 lines)
            for (let i = 0; i <= 5; i++) {
                const y = padding.top + (chartHeight / 5) * i
                ctx.beginPath()
                ctx.moveTo(padding.left, y)
                ctx.lineTo(padding.left + chartWidth, y)
                ctx.stroke()
            }
            
            ctx.setLineDash([])
        }

        function drawThresholdLine(ctx, padding, chartWidth, chartHeight, value, color, label) {
            if (value <= minValue || value >= maxValue) return

            const normalizedY = (maxValue === minValue) ? 0.5 : (value - minValue) / (maxValue - minValue)
            const y = padding.top + chartHeight - (chartHeight * normalizedY)

            ctx.strokeStyle = color
            ctx.lineWidth = 1.0
            ctx.setLineDash([4, 4])
            
            ctx.beginPath()
            ctx.moveTo(padding.left, y)
            ctx.lineTo(padding.left + chartWidth, y)
            ctx.stroke()
            
            ctx.setLineDash([])

            // Threshold label
            ctx.fillStyle = color
            ctx.font = "bold 9px JetBrains Mono"
            ctx.textAlign = "left"
            ctx.fillText(label + ": " + formatValue(value), padding.left + 5, y - 4)
        }
        
        function drawLineChart(ctx, padding, chartWidth, chartHeight) {
            chartData.forEach((series, seriesIndex) => {
                const color = series.color || colorPalette[seriesIndex % colorPalette.length]
                const points = series.data
                const pointCount = points.length
                
                if (pointCount === 0) return
                
                ctx.strokeStyle = color
                ctx.lineWidth = 2
                ctx.lineCap = "round"
                ctx.lineJoin = "round"
                
                ctx.beginPath()
                
                points.forEach((point, i) => {
                    const value = typeof point === "object" ? (point.value || point.y) : point
                    const denominator = Math.max(1, pointCount - 1)
                    const x = padding.left + (chartWidth / denominator) * i
                    const normalizedY = (maxValue === minValue) ? 0.5 : (value - minValue) / (maxValue - minValue)
                    const y = padding.top + chartHeight - (chartHeight * normalizedY * animationProgress)
                    
                    if (i === 0) {
                        ctx.moveTo(x, y)
                    } else {
                        ctx.lineTo(x, y)
                    }
                })
                
                ctx.stroke()
                
                // Draw points
                points.forEach((point, i) => {
                    const value = typeof point === "object" ? (point.value || point.y) : point
                    const denominator = Math.max(1, pointCount - 1)
                    const x = padding.left + (chartWidth / denominator) * i
                    const normalizedY = (maxValue === minValue) ? 0.5 : (value - minValue) / (maxValue - minValue)
                    const y = padding.top + chartHeight - (chartHeight * normalizedY * animationProgress)
                    
                    ctx.beginPath()
                    ctx.arc(x, y, hoveredIndex === i ? 6 : 4, 0, Math.PI * 2)
                    ctx.fillStyle = color
                    ctx.fill()
                    ctx.strokeStyle = backgroundColor
                    ctx.lineWidth = 2
                    ctx.stroke()
                })
            })
        }
        
        function drawBarChart(ctx, padding, chartWidth, chartHeight) {
            const points = chartData[0].data
            const pointCount = points.length
            const barWidth = (chartWidth / pointCount) * 0.7
            const gap = (chartWidth / pointCount) * 0.3
            
            points.forEach((point, i) => {
                const value = typeof point === "object" ? (point.value || point.y) : point
                const color = (typeof point === "object" && point.color) 
                    || colorPalette[i % colorPalette.length]
                
                const x = padding.left + (chartWidth / pointCount) * i + gap / 2
                const normalizedY = (value - minValue) / (maxValue - minValue)
                const barHeight = chartHeight * normalizedY * animationProgress
                const y = padding.top + chartHeight - barHeight
                
                // Bar
                ctx.fillStyle = hoveredIndex === i 
                    ? Qt.lighter(color, 1.2) 
                    : color
                ctx.beginPath()
                ctx.roundRect(x, y, barWidth, barHeight, 4)
                ctx.fill()
            })
        }
        
        function drawAreaChart(ctx, padding, chartWidth, chartHeight) {
            chartData.forEach((series, seriesIndex) => {
                const color = series.color || colorPalette[seriesIndex % colorPalette.length]
                const points = series.data
                const pointCount = points.length
                
                if (pointCount === 0) return
                
                // Area fill
                const gradient = ctx.createLinearGradient(0, padding.top, 0, padding.top + chartHeight)
                gradient.addColorStop(0, Qt.rgba(color.r, color.g, color.b, 0.3))
                gradient.addColorStop(1, Qt.rgba(color.r, color.g, color.b, 0))
                
                ctx.fillStyle = gradient
                ctx.beginPath()
                ctx.moveTo(padding.left, padding.top + chartHeight)
                
                points.forEach((point, i) => {
                    const value = typeof point === "object" ? (point.value || point.y) : point
                    const denominator = Math.max(1, pointCount - 1)
                    const x = padding.left + (chartWidth / denominator) * i
                    const normalizedY = (maxValue === minValue) ? 0.5 : (value - minValue) / (maxValue - minValue)
                    const y = padding.top + chartHeight - (chartHeight * normalizedY * animationProgress)
                    ctx.lineTo(x, y)
                })
                
                ctx.lineTo(padding.left + chartWidth, padding.top + chartHeight)
                ctx.closePath()
                ctx.fill()
                
                // Line on top
                drawLineChart(ctx, padding, chartWidth, chartHeight)
            })
        }
        
        function drawDonutChart(ctx, padding) {
            const centerX = width / 2
            const centerY = height / 2
            const outerRadius = Math.min(width, height) / 2 - 20
            const innerRadius = outerRadius * 0.6
            
            const points = chartData[0].data
            const total = points.reduce((sum, p) => {
                return sum + (typeof p === "object" ? (p.value || 0) : p)
            }, 0)
            
            let currentAngle = -Math.PI / 2
            
            points.forEach((point, i) => {
                const value = typeof point === "object" ? (point.value || point.y) : point
                const color = (typeof point === "object" && point.color) 
                    || colorPalette[i % colorPalette.length]
                
                const sliceAngle = (value / total) * Math.PI * 2 * animationProgress
                
                ctx.fillStyle = hoveredIndex === i 
                    ? Qt.lighter(color, 1.2) 
                    : color
                
                ctx.beginPath()
                ctx.arc(centerX, centerY, outerRadius, currentAngle, currentAngle + sliceAngle)
                ctx.arc(centerX, centerY, innerRadius, currentAngle + sliceAngle, currentAngle, true)
                ctx.closePath()
                ctx.fill()
                
                currentAngle += sliceAngle
            })
        }
        
        function drawAxisLabels(ctx, padding, chartWidth, chartHeight) {
            ctx.fillStyle = textColor
            ctx.font = "12px sans-serif"
            ctx.textAlign = "right"
            
            // Y-axis labels
            for (let i = 0; i <= 5; i++) {
                const value = minValue + ((maxValue - minValue) / 5) * (5 - i)
                const y = padding.top + (chartHeight / 5) * i + 4
                ctx.fillText(formatValue(value), padding.left - 8, y)
            }
            
            // X-axis labels
            ctx.textAlign = "center"
            const points = chartData[0].data
            const step = Math.ceil(points.length / 6)  // Max 6 labels
            
            points.forEach((point, i) => {
                if (i % step !== 0) return
                
                const label = typeof point === "object" 
                    ? (point.label || point.x || i) 
                    : i
                const denominator = Math.max(1, points.length - 1)
                const x = padding.left + (chartWidth / denominator) * i
                ctx.fillText(String(label), x, padding.top + chartHeight + 20)
            })
        }
        
        function formatValue(value) {
            if (value >= 1000000) return (value / 1000000).toFixed(1) + "M"
            if (value >= 1000) return (value / 1000).toFixed(1) + "K"
            return value.toFixed(0)
        }
        
        // Redraw on data change
        Connections {
            target: root
            function onDataChanged() { canvas.requestPaint() }
            function onSeriesChanged() { canvas.requestPaint() }
            function onAnimationProgressChanged() { canvas.requestPaint() }
            function onHoveredIndexChanged() { canvas.requestPaint() }
        }
        
        // Mouse tracking for tooltips
        MouseArea {
            anchors.fill: parent
            hoverEnabled: showTooltip
            
            onPositionChanged: {
                mousePos = Qt.point(mouse.x, mouse.y)
                
                // Calculate hovered index
                const padding = { left: 50 }
                const chartWidth = canvas.width - 70
                const points = chartData[0].data.length
                
                if (type === "donut") {
                    // TODO: Donut hover detection
                } else {
                    const relX = mouse.x - padding.left
                    const idx = Math.round((relX / chartWidth) * (points - 1))
                    hoveredIndex = (idx >= 0 && idx < points) ? idx : -1
                }
            }
            
            onExited: {
                hoveredIndex = -1
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // TOOLTIP
    // ═══════════════════════════════════════════════════════════════
    
    Rectangle {
        id: tooltip
        visible: showTooltip && hoveredIndex >= 0
        
        x: Math.min(mousePos.x + 10, root.width - width - 10)
        y: Math.max(mousePos.y - height - 10, 10)
        
        width: tooltipContent.width + 16
        height: tooltipContent.height + 12
        radius: 4
        color: "#1E293B"
        border.color: gridColor
        
        Column {
            id: tooltipContent
            anchors.centerIn: parent
            spacing: 4
            
            Text {
                text: {
                    if (hoveredIndex < 0) return ""
                    const point = chartData[0].data[hoveredIndex]
                    return typeof point === "object" 
                        ? (point.label || "Point " + hoveredIndex)
                        : "Point " + hoveredIndex
                }
                font.pixelSize: 11
                font.weight: Font.Medium
                color: textColor
            }
            
            Text {
                text: {
                    if (hoveredIndex < 0) return ""
                    const point = chartData[0].data[hoveredIndex]
                    const value = typeof point === "object" 
                        ? (point.value || point.y) 
                        : point
                    return "Value: " + Number(value).toLocaleString()
                }
                font.pixelSize: 12
                color: Qt.lighter(textColor, 1.5)
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // LEGEND
    // ═══════════════════════════════════════════════════════════════
    
    Row {
        visible: showLegend && chartData.length > 1
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 8
        spacing: 16
        
        Repeater {
            model: chartData
            
            Row {
                spacing: 6
                
                Rectangle {
                    width: 12
                    height: 12
                    radius: 2
                    color: modelData.color || colorPalette[index % colorPalette.length]
                }
                
                Text {
                    text: modelData.name || ("Series " + (index + 1))
                    font.pixelSize: 12
                    color: textColor
                }
            }
        }
    }
}
