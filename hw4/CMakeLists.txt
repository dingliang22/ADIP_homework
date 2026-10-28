find_path(HW4_STB_INCLUDE_DIR NAMES stb_image_write.h PATH_SUFFIXES stb REQUIRED)
add_library(hw4_common STATIC
    src/common/path.cpp
    src/common/raw_io.cpp
    src/common/png_output.cpp
    src/common/filters.cpp
    src/common/interpolation.cpp
    src/common/edges.cpp
    src/common/metrics.cpp
    src/common/experiments.cpp
)
target_compile_features(hw4_common PUBLIC cxx_std_20)
target_include_directories(hw4_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(hw4_common SYSTEM PRIVATE ${HW4_STB_INCLUDE_DIR})
target_compile_definitions(hw4_common PRIVATE HW4_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_library(hw4_all STATIC
    src/hw4/hw4_1_1.cpp src/hw4/hw4_1_2.cpp src/hw4/hw4_1_3.cpp src/hw4/hw4_1_4.cpp
    src/hw4/hw4_2_1.cpp src/hw4/hw4_2_2.cpp
    src/hw4/hw4_3_1.cpp src/hw4/hw4_3_2.cpp src/hw4/hw4_3_3.cpp src/hw4/hw4_3_4.cpp
)
target_link_libraries(hw4_all PUBLIC hw4_common)
add_executable(hw4_main main.cpp)
target_link_libraries(hw4_main PRIVATE hw4_all)


