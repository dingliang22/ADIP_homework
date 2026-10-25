#pragma once

#include "hw3/common.hpp"

#include <string>
#include <vector>

namespace hw3 {

    GrayImage embedLSB(const GrayImage& image, const std::string& studentId);

    std::vector<Pixel> decodeLSBBits(const GrayImage& image);

    void runHw3_4_1(const fs::path& inputFolder, const fs::path& outputRoot,
                    const std::string& studentId);

} // namespace hw3