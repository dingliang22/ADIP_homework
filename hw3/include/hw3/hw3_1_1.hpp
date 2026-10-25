#pragma once

#include "hw3/common.hpp"

namespace hw3 {

    // Global Histogram Equalization
    GrayImage histogramEqualization(const GrayImage& image);

    // Entry point for HW3 1-1
    void runHw3_1_1(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3