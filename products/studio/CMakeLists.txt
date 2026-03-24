# ─── AXIOM Studio ─────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 OpenGL)

add_executable(axiom_studio
    src/main.cpp
    ${CMAKE_SOURCE_DIR}/axui/qml/resources/axui_dashboard.qrc
    # ${CMAKE_SOURCE_DIR}/axui/qml/resources/axui_studio.qrc # add if it exists
)

target_link_libraries(axiom_studio PRIVATE
    axui_qml
    axui_runtime
    axiom_engine
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::OpenGL
)

if(WIN32)
    target_link_libraries(axiom_studio PRIVATE d3d11 dxgi dxguid d3d12)
endif()

target_include_directories(axiom_studio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET axiom_studio PROPERTY CXX_STANDARD 20)
