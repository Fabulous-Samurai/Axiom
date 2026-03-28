# ─── AXIOM Dashboard Product ──────────────────────────────────────
cmake_minimum_required(VERSION 3.20)
project(axiom_dashboard LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 OpenGL)

set(DASHBOARD_SOURCES
    src/dashboard_main.cpp
)

add_executable(axiom_dashboard ${DASHBOARD_SOURCES})

# QML Resources (Only product-specific files)
qt_add_resources(axiom_dashboard "dashboard_qml"
    PREFIX "/"
    FILES
        ui/AxiomDashboard.qml
        ui/components/FlowNode.qml
        ui/components/FlowLink.qml
        ui/components/TelemetryGauge.qml
        ui/components/DetailPanel.qml
)

target_link_libraries(axiom_dashboard PRIVATE
    axui_qml
    axui_runtime
    axiom_engine
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::OpenGL
)

if(WIN32)
    target_link_libraries(axiom_dashboard PRIVATE d3d11 dxgi dxguid d3d12)
endif()

target_include_directories(axiom_dashboard PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/engine/include
)

set_property(TARGET axiom_dashboard PROPERTY CXX_STANDARD 20)
