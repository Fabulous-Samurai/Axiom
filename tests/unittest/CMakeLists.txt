cmake_minimum_required(VERSION 3.12)

project(axiom_unittest)

set(TEST_SOURCES
    sample_test.cpp
    test_eigen_engine.cpp
    ../detailed_hardware_alignment_test.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/include)

axiom_add_gtest(axiom_unit_tests "${TEST_SOURCES}")
