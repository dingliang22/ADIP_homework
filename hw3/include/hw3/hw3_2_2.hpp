#pragma once

#include "hw3/common.hpp"

namespace hw3 {

    // Power-law (Gamma) Transformation
    GrayImage powerLawTransformation(const GrayImage& image, double gamma);

    // Entry point for HW3 2-2
    void runHw3_2_2(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3