#pragma once

#include "hw3/common.hpp"

#include <string>
#include <vector>

namespace hw3 {

    void validateStudentId(const std::string& studentId);

    Pixel repeatedIdBit(const std::string& studentId, size_t index);

    struct DecodeReport {
        size_t bitErrors = 0;
        size_t charErrors = 0;
        size_t totalBits = 0;

        std::string asciiPreview;

        bool firstIdCorrect = false;
    };

    DecodeReport analyzeDecodedBits(const std::vector<Pixel>& bits, const std::string& studentId);

    void printDecodeReport(const std::string& label, const DecodeReport& report);

} // namespace hw3