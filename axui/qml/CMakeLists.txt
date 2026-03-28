cmake_minimum_required(VERSION 3.16)

# Qt6 is found at root CMakeLists.txt — no need to find_package again


# Qt MOC/RCC/UIC automation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ─── QML Loader Library ───────────────────────────────────────────
add_library(axui_qml STATIC
    src/qml_loader.cpp
    src/component_registry.cpp
    include/axui/qml_loader.h
    include/axui/component_registry.h
    resources/axui_components.qrc
)

target_include_directories(axui_qml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(axui_qml PUBLIC
    axui_compiler
    axui_runtime
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::OpenGL
)

set_property(TARGET axui_qml PROPERTY CXX_STANDARD 23)

# ─── AXUI Viewer Application ──────────────────────────────────────
add_executable(axui_viewer
    src/main.cpp
)

target_link_libraries(axui_viewer PRIVATE
    axui_qml
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

set_property(TARGET axui_viewer PROPERTY CXX_STANDARD 20)

# ─── Tests ────────────────────────────────────────────────────────
if(AXUI_BUILD_TESTS)
    add_executable(axui_qml_tests
        tests/test_qml_loader.cpp
        tests/test_qml_main.cpp
    )

    target_link_libraries(axui_qml_tests PRIVATE
        axui_qml
        Qt6::Test
    )

    set_property(TARGET axui_qml_tests PROPERTY CXX_STANDARD 20)
    add_test(NAME AxuiQmlTests COMMAND axui_qml_tests)

    add_executable(axui_component_tests
        tests/test_components.cpp
    )

    target_link_libraries(axui_component_tests PRIVATE
        axui_qml
        Qt6::Test
    )

    set_property(TARGET axui_component_tests PROPERTY CXX_STANDARD 20)
    add_test(NAME AxuiComponentTests COMMAND axui_component_tests)
endif()
