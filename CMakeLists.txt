cmake_minimum_required(VERSION 3.18)
project(adip_homeworks LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 5.0 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})
add_subdirectory(hw1)
add_subdirectory(hw2)
add_subdirectory(hw3)
add_subdirectory(hw4)