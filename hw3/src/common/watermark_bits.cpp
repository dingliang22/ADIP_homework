#include "hw3/watermark_bits.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace hw3 {
    // Validate student ID.
    void validateStudentId(const std::string& id) {
        if (id.empty()) {
            throw std::invalid_argument("Student ID is empty.");
        }
        for (char c : id) {
            if (c < '0' || c > '9') {
                throw std::invalid_argument("Student ID must contain digits only.");
            }
        }
    }

    // Get repeated ASCII bit, starting from each character's MSB.
    Pixel repeatedIdBit(const std::string& id, size_t index) {
        const size_t characterIndex = (index / 8) % id.size();
        const int position = 7 - static_cast<int>(index % 8);
        return static_cast<Pixel>((static_cast<unsigned char>(id[characterIndex]) >> position) &
                                  1u);
    }

    // Compare decoded bits with the repeated student ID.
    DecodeReport analyzeDecodedBits(const std::vector<Pixel>& bits, const std::string& id) {
        validateStudentId(id);
        DecodeReport report;
        report.totalBits = bits.size();
        const size_t charCount = bits.size() / 8;
        const size_t previewCount = std::min<size_t>(27, charCount);
        std::string firstId;
        // Count bit errors.
        for (size_t i = 0; i < bits.size(); ++i) {
            if ((bits[i] & 1u) != repeatedIdBit(id, i)) {
                ++report.bitErrors;
            }
        }
        // Decode ASCII characters.
        for (size_t i = 0; i < charCount; ++i) {
            unsigned char c = 0;
            for (int bit = 0; bit < 8; ++bit) {
                c = static_cast<unsigned char>((c << 1) | (bits[i * 8 + bit] & 1u));
            }
            const unsigned char expected = static_cast<unsigned char>(id[i % id.size()]);
            if (c != expected) {
                ++report.charErrors;
            }
            // Show printable ASCII only.
            if (i < previewCount) {
                report.asciiPreview.push_back(c >= 32 && c <= 126 ? static_cast<char>(c) : '.');
            }
            if (i < id.size()) {
                firstId.push_back(static_cast<char>(c));
            }
        }
        report.firstIdCorrect = firstId == id;
        return report;
    }

    // Print BER, character errors and decoded content.
    void printDecodeReport(const std::string& label, const DecodeReport& report) {
        const double ber =
            report.totalBits ? static_cast<double>(report.bitErrors) / report.totalBits : 0.0;
        const size_t chars = report.totalBits / 8;
        const double cer = chars ? static_cast<double>(report.charErrors) / chars : 0.0;
        std::cout << label << '\n';
        std::cout << "  First ID correct: " << (report.firstIdCorrect ? "YES" : "NO") << '\n';
        std::cout << "  ASCII preview   : " << report.asciiPreview << '\n';
        std::cout << "  Bit errors      : " << report.bitErrors << "/" << report.totalBits << '\n';
        std::cout << "  BER             : " << std::fixed << std::setprecision(6) << ber << '\n';
        std::cout << "  Character errors: " << report.charErrors << "/" << chars << '\n';
        std::cout << "  CER             : " << std::fixed << std::setprecision(6) << cer << '\n';
    }
} // namespace hw3