cmake_minimum_required(VERSION 3.5...4.0)
project(tetris VERSION 1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE tetris.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601)

if(NOT WIN32)
    message(FATAL_ERROR "This project can only be built on Windows.")
endif()
