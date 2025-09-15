cmake_minimum_required(VERSION 3.20)
project(rpn_tests)

# Используем системный Google Test
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

add_executable(rpn_tests test_rpn.cpp ../src/rpn.cpp)

target_link_libraries(rpn_tests PRIVATE 
    GTest::gtest_main
    GTest::gtest
    Threads::Threads
)

target_include_directories(rpn_tests PRIVATE ../include)

enable_testing()
add_test(NAME rpn_tests COMMAND rpn_tests)