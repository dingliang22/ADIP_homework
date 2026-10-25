#pragma once

#include "hw3/common.hpp"

namespace hw3 {

    // Negative Gray-Level Transformation
    GrayImage negativeTransformation(const GrayImage& image);

    // Entry point for HW3 2-1
    void runHw3_2_1(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3