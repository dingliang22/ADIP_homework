if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.18)
    project(hw6 LANGUAGES CXX)
endif()

find_path(HW6_STB_INCLUDE_DIR NAMES stb_image_write.h PATH_SUFFIXES stb REQUIRED)

# Shared utilities: RAW/PNG I/O, own morphology (erosion/dilation/opening/closing/top-hat/
# reconstruction) and Zhang-Suen thinning + crossing number. No OpenCV calls.
add_library(hw6_common STATIC
    src/common/path.cpp
    src/common/raw_io.cpp
    src/common/png_output.cpp
    src/common/morphology.cpp
    src/common/thinning.cpp
)
target_compile_features(hw6_common PUBLIC cxx_std_20)
target_include_directories(hw6_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(hw6_common SYSTEM PRIVATE ${HW6_STB_INCLUDE_DIR})
target_compile_definitions(hw6_common PRIVATE HW6_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_library(hw6_all STATIC
    src/hw6/hw6_1_1.cpp src/hw6/hw6_1_2.cpp
    src/hw6/hw6_2_1.cpp src/hw6/hw6_2_2.cpp
    src/hw6/hw6_3.cpp
)
target_link_libraries(hw6_all PUBLIC hw6_common)
add_executable(hw6_main main.cpp)
target_link_libraries(hw6_main PRIVATE hw6_all)
