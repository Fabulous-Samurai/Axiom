# ─── AXIOM Studio Product ─────────────────────────────────────────
cmake_minimum_required(VERSION 3.20)
project(axiom_studio LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 is found at root CMakeLists.txt — no need to find_package again


set(STUDIO_SOURCES
    src/main.cpp
    src/config.py
    src/version.py
)

add_executable(axiom_studio ${STUDIO_SOURCES})

# QML Resources
qt_add_resources(axiom_studio "studio_qml"
    PREFIX "/"
    FILES
        ui/main.qml
        ui/Recovery.qml
        ui/components/SidebarItem.qml
        ui/components/TelemetryBadge.qml
        ui/components/qmldir
        ui/theme/qmldir
        ui/theme/Colors.qml
        ui/theme/Elevation.qml
        ui/theme/Spacing.qml
        ui/theme/Typography.qml
)

target_link_libraries(axiom_studio PRIVATE
    axui_qml
    axui_runtime
    axui_widgets
    axiom_engine
    axiom_expressway
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::OpenGL
)

if(WIN32)
    target_link_libraries(axiom_studio PRIVATE d3d11 dxgi dxguid d3d12)
endif()

target_include_directories(axiom_studio PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/engine/include
)

set_property(TARGET axiom_studio PROPERTY CXX_STANDARD 23)

# Output binary to a predictable location
set_target_properties(axiom_studio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
