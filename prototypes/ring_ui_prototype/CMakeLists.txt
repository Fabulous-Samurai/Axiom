cmake_minimum_required(VERSION 3.16)
project(ring_ui_prototype VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 COMPONENTS Core Qml Quick REQUIRED)
find_package(axui CONFIG REQUIRED) # Assuming axui is installed via CMake or findable

# Add the executable for the prototype
add_executable(ring_ui_prototype
    main.cpp
)

# Link against axui_core (for AppRunner) and other Qt components
target_link_libraries(ring_ui_prototype PRIVATE
    axui_core
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
)

# Setup QML resource compilation
qt_add_qml_module(ring_ui_prototype
    URI AxiomRingUI
    VERSION 1.0
    QML_FILES
        main.qml
        CircularDashboard.qml # Placeholder for the ring component
)

# Automatically register QML types from the module
qt_target_qml_sources(ring_ui_prototype PRIVATE AxiomRingUI)

# Ensure resources are handled
qt_add_resources(ring_ui_prototype "ring_ui_prototype_resources.qrc")
