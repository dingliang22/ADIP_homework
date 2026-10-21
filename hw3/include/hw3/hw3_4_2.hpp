#pragma once

#include "hw3/common.hpp"

#include <string>
#include <vector>

namespace hw3 {

    GrayImage embedBlockMedianQIM(const GrayImage& image, const std::string& studentId, int delta);

    std::vector<Pixel> decodeQIMBits(const GrayImage& image, int delta);

    void runHw3_4_2(const fs::path& inputFolder, const fs::path& outputRoot,
                    const std::string& studentId);

} // namespace hw3