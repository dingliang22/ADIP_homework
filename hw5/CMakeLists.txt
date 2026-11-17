if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.18)
    project(hw5 LANGUAGES CXX)
endif()

find_package(OpenCV 5.0 REQUIRED COMPONENTS core)
find_path(HW5_STB_INCLUDE_DIR NAMES stb_image_write.h PATH_SUFFIXES stb REQUIRED)

# Shared utilities: RAW/PNG I/O, metrics, resize, Fourier/DCT helpers, hashing, JPEG.
# OpenCV is used only for cv::dft / cv::idft (Q1-2, Q1-3) and cv::dct / cv::idct (Q2, Q3).
add_library(hw5_common STATIC
    src/common/path.cpp
    src/common/raw_io.cpp
    src/common/png_output.cpp
    src/common/metrics.cpp
    src/common/resize.cpp
    src/common/fourier.cpp
    src/common/freq_filters.cpp
    src/common/hashing.cpp
    src/common/jpeg.cpp
)
target_compile_features(hw5_common PUBLIC cxx_std_20)
target_include_directories(hw5_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_include_directories(hw5_common SYSTEM PRIVATE ${HW5_STB_INCLUDE_DIR})
target_link_libraries(hw5_common PUBLIC ${OpenCV_LIBS})
target_compile_definitions(hw5_common PRIVATE HW5_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_library(hw5_all STATIC
    src/hw5/hw5_1_1.cpp src/hw5/hw5_1_2.cpp src/hw5/hw5_1_3.cpp
    src/hw5/hw5_2_2.cpp src/hw5/hw5_2_3.cpp src/hw5/hw5_2_4.cpp
    src/hw5/hw5_3_1.cpp src/hw5/hw5_3_2.cpp src/hw5/hw5_3_3.cpp src/hw5/hw5_3_4.cpp
)
target_link_libraries(hw5_all PUBLIC hw5_common)
add_executable(hw5_main main.cpp)
target_link_libraries(hw5_main PRIVATE hw5_all)
