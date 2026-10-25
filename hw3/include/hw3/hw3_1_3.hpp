#pragma once

#include "hw3/common.hpp"

namespace hw3 {

    // Histogram Matching
    // Match the source image histogram to the reference image.
    GrayImage histogramMatching(const GrayImage& source, const GrayImage& reference);

    // Entry point for HW3 1-3
    void runHw3_1_3(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3