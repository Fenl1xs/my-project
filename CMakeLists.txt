cmake_minimum_required(VERSION 3.20)
project(RPN_Calculator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Основное приложение
include_directories(include)
add_executable(rpn_calculator src/main.cpp src/rpn.cpp)

# Включение тестов
enable_testing()
add_subdirectory(tests)