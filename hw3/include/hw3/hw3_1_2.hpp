#pragma once

#include "hw3/common.hpp"

namespace hw3 {

    // Local Histogram Equalization
    GrayImage localHistogramEqualization(const GrayImage& image, int windowSize);

    // Entry point for HW3 1-2
    void runHw3_1_2(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3