pragma Singleton
import QtQuick

QtObject {
    readonly property var level0: ({})
    readonly property var level1: ({
        "color": "#1E000000",
        "horizontalOffset": 0,
        "verticalOffset": 2,
        "radius": 4,
        "samples": 17
    })
    readonly property var level2: ({
        "color": "#26000000",
        "horizontalOffset": 0,
        "verticalOffset": 4,
        "radius": 8,
        "samples": 17
    })
    readonly property var level3: ({
        "color": "#33000000",
        "horizontalOffset": 0,
        "verticalOffset": 8,
        "radius": 16,
        "samples": 17
    })
    readonly property var level4: ({
        "color": "#40000000",
        "horizontalOffset": 0,
        "verticalOffset": 16,
        "radius": 32,
        "samples": 17
    })
}
