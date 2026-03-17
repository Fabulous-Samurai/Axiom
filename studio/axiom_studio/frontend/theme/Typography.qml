pragma Singleton
import QtQuick

QtObject {
    readonly property string heading:   "JetBrains Mono"
    readonly property string body:      "Inter"
    readonly property string code:      "JetBrains Mono"

    readonly property int h1:           32
    readonly property int h2:           25
    readonly property int h3:           20
    readonly property int h4:           16
    readonly property int bodyLarge:    15
    readonly property int bodyDefault:  13
    readonly property int small:        12
    readonly property int xsmall:       11

    readonly property int light:        300
    readonly property int regular:      400
    readonly property int medium:       500
    readonly property int semibold:     600
    readonly property int bold:         700

    readonly property int metric:       28
    readonly property int metricLarge:  32

    readonly property real headingLineHeight:  1.2
    readonly property real bodyLineHeight:     1.6

    readonly property real headingSpacing:    -0.02
    readonly property real bodySpacing:        0.0
}