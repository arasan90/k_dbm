cmake_minimum_required(VERSION 3.10)
include(GoogleTest)

project(k_dbm_test LANGUAGES C CXX VERSION 1.0.0)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/k_dbm_test.cpp)
target_link_libraries(${PROJECT_NAME} gtest gtest_main k_dbm)
target_include_directories(${PROJECT_NAME} PRIVATE ../src)

gtest_discover_tests(${PROJECT_NAME})
