cmake_minimum_required(VERSION 3.12)

project(axiom_unittest)

set(TEST_SOURCES
    sample_test.cpp
    ../detailed_hardware_alignment_test.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/engine/include)

axiom_add_gtest(axiom_unit_tests "${TEST_SOURCES}")

if(WIN32)
    set_target_properties(axiom_unit_tests PROPERTIES WIN32_EXECUTABLE FALSE)
    target_link_options(axiom_unit_tests PRIVATE "-mconsole" "-Wl,-e,mainCRTStartup")
endif()

target_link_libraries(axiom_unit_tests PRIVATE
    axiom_engine_ipc
    axiom_engine_lib
    axiom_engine_core
)
