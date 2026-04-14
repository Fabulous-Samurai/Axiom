import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

/*
 * AXKPICard - Key Performance Indicator Card
 *
 * Usage in .axui:
 * {
 *   "component": "KPICard",
 *   "props": {
 *     "title": "Revenue",
 *     "value": "@engine.metrics.revenue",
 *     "trend": 12.5,
 *     "trendDirection": "up",
 *     "icon": "currency-dollar"
 *   },
 *   "glass": { "enabled": true, "blur": 12 }
 * }
 */

Rectangle {
    id: root

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    // Required
    property string title: "KPI"
    property var value: 0  // Can be number or string

    // Optional
    property real trend: 0.0
    property string trendDirection: "up"  // "up", "down", "neutral"
    property string icon: ""
    property string unit: ""
    property string subtitle: ""

    // Theme properties (bound from @colors.*)
    property color cardBackground: "#1E293B"
    property color cardBorder: "#334155"
    property color titleColor: "#94A3B8"
    property color valueColor: "#F8FAFC"
    property color trendUpColor: "#22C55E"
    property color trendDownColor: "#EF4444"
    property color trendNeutralColor: "#94A3B8"

    // Glass effect
    property bool glassEnabled: false
    property real glassBlur: 12
    property real glassOpacity: 0.1
    property real glassNoiseStrength: 0.02

    // Hover effect
    property bool hoverEnabled: true
    property real hoverScale: 1.02
    property real hoverGlowRadius: 8
    property color hoverGlowColor: "#3B82F6"

    // Animation
    property int animationDuration: 200
    property int valueAnimationDuration: 800

    // Sizing
    property real minWidth: 200
    property real minHeight: 120

    // ═══════════════════════════════════════════════════════════════
    // INTERNAL STATE
    // ═══════════════════════════════════════════════════════════════

    implicitWidth: Math.max(minWidth, contentLayout.implicitWidth + 32)
    implicitHeight: Math.max(minHeight, contentLayout.implicitHeight + 32)

    radius: 12
    color: glassEnabled ? Qt.rgba(cardBackground.r, cardBackground.g,
                                   cardBackground.b, glassOpacity)
                        : cardBackground
    border.color: cardBorder
    border.width: 1

    // Accessibility
    Accessible.role: Accessible.StaticText
    Accessible.name: title + ": " + displayValue
    Accessible.description: subtitle + " Trend: " + trend + "%"

    // ═══════════════════════════════════════════════════════════════
    // COMPUTED PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    readonly property string displayValue: {
        if (typeof value === "number") {
            return formatNumber(value) + unit
        }
        return String(value) + unit
    }

    readonly property color currentTrendColor: {
        if (trendDirection === "up") return trendUpColor
        if (trendDirection === "down") return trendDownColor
        return trendNeutralColor
    }

    readonly property string trendIcon: {
        if (trendDirection === "up") return "▲"
        if (trendDirection === "down") return "▼"
        return "●"
    }

    // ═══════════════════════════════════════════════════════════════
    // ANIMATIONS
    // ═══════════════════════════════════════════════════════════════

    scale: 1.0

    Behavior on scale {
        NumberAnimation {
            duration: animationDuration
            easing.type: Easing.OutCubic
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: animationDuration
        }
    }

    // Value change animation
    property real animatedValue: 0

    Behavior on animatedValue {
        NumberAnimation {
            duration: valueAnimationDuration
            easing.type: Easing.OutExpo
        }
    }

    onValueChanged: {
        if (typeof value === "number") {
            animatedValue = value
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // GLASS EFFECT (Blur + Noise)
    // ═══════════════════════════════════════════════════════════════

    layer.enabled: glassEnabled
    layer.effect: ShaderEffect {
        property real blurRadius: root.glassBlur
        property real noiseStrength: root.glassNoiseStrength
        property real time: 0

        NumberAnimation on time {
            from: 0
            to: 1
            duration: 10000
            loops: Animation.Infinite
        }

        fragmentShader: "
            varying highp vec2 qt_TexCoord0;
            uniform sampler2D source;
            uniform lowp float blurRadius;
            uniform lowp float noiseStrength;
            uniform lowp float time;

            // Simple hash function for noise
            highp float hash(highp vec2 p) {
                return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
            }

            void main() {
                // Box blur (simplified)
                vec4 color = vec4(0.0);
                float total = 0.0;
                float radius = blurRadius / 500.0;

                for (float x = -2.0; x <= 2.0; x += 1.0) {
                    for (float y = -2.0; y <= 2.0; y += 1.0) {
                        vec2 offset = vec2(x, y) * radius;
                        color += texture2D(source, qt_TexCoord0 + offset);
                        total += 1.0;
                    }
                }
                color /= total;

                // Add subtle noise
                float noise = hash(qt_TexCoord0 * 100.0 + time) * noiseStrength;
                color.rgb += vec3(noise);

                gl_FragColor = color;
            }
        "
    }

    // ═══════════════════════════════════════════════════════════════
    // HOVER GLOW EFFECT
    // ═══════════════════════════════════════════════════════════════

    Rectangle {
        id: glowEffect
        anchors.fill: parent
        anchors.margins: -hoverGlowRadius
        radius: parent.radius + hoverGlowRadius
        color: "transparent"
        opacity: 0

        border.color: hoverGlowColor
        border.width: 2

        Behavior on opacity {
            NumberAnimation { duration: animationDuration }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // MOUSE INTERACTION
    // ═══════════════════════════════════════════════════════════════

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.hoverEnabled
        cursorShape: Qt.PointingHandCursor

        onEntered: {
            if (hoverEnabled) {
                root.scale = hoverScale
                glowEffect.opacity = 0.5
            }
        }

        onExited: {
            root.scale = 1.0
            glowEffect.opacity = 0
        }

        onClicked: {
            root.clicked()
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // SIGNALS
    // ═══════════════════════════════════════════════════════════════

    signal clicked()

    // ═══════════════════════════════════════════════════════════════
    // CONTENT LAYOUT
    // ═══════════════════════════════════════════════════════════════

    ColumnLayout {
        id: contentLayout
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        // Header row (icon + title)
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Icon (if provided)
            Text {
                id: iconText
                visible: root.icon !== ""
                text: getIconGlyph(root.icon)
                font.family: "Material Icons"  // Or your icon font
                font.pixelSize: 20
                color: root.titleColor
            }

            // Title
            Text {
                id: titleText
                Layout.fillWidth: true
                text: root.title
                font.pixelSize: 14
                font.weight: Font.Medium
                color: root.titleColor
                elide: Text.ElideRight
            }

            // Trend badge
            Rectangle {
                visible: root.trend !== 0
                implicitWidth: trendRow.implicitWidth + 12
                implicitHeight: 24
                radius: 4
                color: Qt.rgba(currentTrendColor.r, currentTrendColor.g,
                               currentTrendColor.b, 0.15)

                RowLayout {
                    id: trendRow
                    anchors.centerIn: parent
                    spacing: 4

                    Text {
                        text: root.trendIcon
                        font.pixelSize: 10
                        color: root.currentTrendColor
                    }

                    Text {
                        text: Math.abs(root.trend).toFixed(1) + "%"
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        color: root.currentTrendColor
                    }
                }
            }
        }

        // Spacer
        Item { Layout.fillHeight: true }

        // Value (large, animated)
        Text {
            id: valueText
            Layout.fillWidth: true
            text: typeof value === "number"
                  ? formatNumber(animatedValue) + unit
                  : displayValue
            font.pixelSize: 32
            font.weight: Font.Bold
            color: root.valueColor
            elide: Text.ElideRight

            // Sparkle effect on value change
            SequentialAnimation {
                id: valueChangeAnimation

                NumberAnimation {
                    target: valueText
                    property: "opacity"
                    to: 0.5
                    duration: 100
                }
                NumberAnimation {
                    target: valueText
                    property: "opacity"
                    to: 1.0
                    duration: 200
                }
            }
        }

        // Subtitle (optional)
        Text {
            visible: root.subtitle !== ""
            Layout.fillWidth: true
            text: root.subtitle
            font.pixelSize: 12
            color: Qt.rgba(root.titleColor.r, root.titleColor.g,
                          root.titleColor.b, 0.7)
            elide: Text.ElideRight
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // HELPER FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    function formatNumber(num) {
        if (num >= 1000000000) {
            return (num / 1000000000).toFixed(1) + "B"
        } else if (num >= 1000000) {
            return (num / 1000000).toFixed(1) + "M"
        } else if (num >= 1000) {
            return (num / 1000).toFixed(1) + "K"
        }
        return num.toFixed(0)
    }

    function getIconGlyph(iconName) {
        // Icon mapping (Material Icons veya custom)
        const icons = {
            "currency-dollar": "$",
            "users": "👥",
            "chart-line": "📈",
            "clock": "🕐",
            "check-circle": "✓",
            "alert-triangle": "⚠",
            "arrow-up": "↑",
            "arrow-down": "↓"
        }
        return icons[iconName] || "●"
    }

    // ═══════════════════════════════════════════════════════════════
    // LIFECYCLE
    // ═══════════════════════════════════════════════════════════════

    Component.onCompleted: {
        // Initial value animation
        if (typeof value === "number") {
            animatedValue = 0
            animatedValue = value
        }
    }
}
