cmake_minimum_required(VERSION 3.5...4.0)
project(tetris VERSION 1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE tetris.cpp)

if(NOT WIN32)
    message(FATAL_ERROR "This project can only be built on Windows.")
endif()