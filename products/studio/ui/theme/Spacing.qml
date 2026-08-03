pragma Singleton
import QtQuick

QtObject {
    readonly property int micro:        4
    readonly property int small:        8
    readonly property int mediumSmall:  12
    readonly property int medium:       16
    readonly property int mediumLarge:  20
    readonly property int large:        24
    readonly property int xlarge:       32
    readonly property int xxlarge:      48

    readonly property int radiusSmall:  4
    readonly property int radiusMedium: 8
    readonly property int radiusLarge:  12
    readonly property int radiusXL:     16
    readonly property int radiusPill:   9999

    readonly property int sidebarExpanded:   220
    readonly property int sidebarCollapsed:  60

    readonly property int navbarHeight:      64

    readonly property int breakpointMobile:  768
    readonly property int breakpointTablet:  992
    readonly property int breakpointDesktop: 1200
}
