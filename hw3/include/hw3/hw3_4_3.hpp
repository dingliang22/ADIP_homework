#pragma once

#include "hw3/common.hpp"

#include <string>

namespace hw3 {

    GrayImage quantizeTo6Bit(const GrayImage& image);

    void runHw3_4_3(const fs::path& inputFolder, const fs::path& outputRoot,
                    const std::string& studentId);

} // namespace hw3