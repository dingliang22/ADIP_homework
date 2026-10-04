cmake_minimum_required(VERSION 3.10)
project(adip_homeworks)

find_package(OpenCV 5.0 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_libraries(${OpenCV_LIBS})
add_subdirectory(hw1)
add_subdirectory(hw2)