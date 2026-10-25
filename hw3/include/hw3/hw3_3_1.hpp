#pragma once

#include "hw3/common.hpp"

namespace hw3 {

    // Keep only the highest k bit-planes.
    // k must be in [1, 8].
    GrayImage keepTopKPlanes(const GrayImage& image, int k);

    // Entry point for HW3 3-1.
    void runHw3_3_1(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3