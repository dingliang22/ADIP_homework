# 通用函式
add_library(hw3_common STATIC
    src/common/path.cpp
    src/common/raw_io.cpp
    src/common/histogram.cpp
    src/common/metrics.cpp
    src/common/png_output.cpp
)
target_include_directories(hw3_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(hw3_common
    PUBLIC
        ${OpenCV_LIBS}
)
# 所有題目實做
add_library(hw3_all STATIC
    src/common/watermark_bits.cpp
    src/hw3/hw3_1_1.cpp
    src/hw3/hw3_1_2.cpp
    src/hw3/hw3_1_3.cpp
    src/hw3/hw3_2_1.cpp
    src/hw3/hw3_2_2.cpp
    src/hw3/hw3_3_1.cpp
    src/hw3/hw3_3_2.cpp
    src/hw3/hw3_4_1.cpp
    src/hw3/hw3_4_2.cpp
    src/hw3/hw3_4_3.cpp
)

target_link_libraries(hw3_all PUBLIC hw3_common)

add_executable(hw3_main main.cpp)

target_link_libraries(hw3_main PRIVATE hw3_all)