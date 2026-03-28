set(CMAKE_AUTOMOC ON)

add_library(axui_widgets STATIC
    throughput_node.cpp
    throughput_node.h
    pluto_navigator.cpp
    pluto_navigator.h
)

target_include_directories(axui_widgets PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/include
)

target_link_libraries(axui_widgets PUBLIC
    axiom_engine
    Qt6::Core
    Qt6::Quick
)

set_property(TARGET axui_widgets PROPERTY CXX_STANDARD 20)
