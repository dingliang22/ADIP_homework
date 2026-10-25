#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace hw3 {

    namespace fs = std::filesystem;

    using Pixel = uint8_t;
    using Histogram = std::array<uint32_t, 256>;

    struct GrayImage {
        int width = 0;
        int height = 0;
        std::vector<Pixel> pixels;

        bool valid() const {
            return width > 0 && height > 0 &&
                   pixels.size() == static_cast<size_t>(width) * static_cast<size_t>(height);
        }
    };

    // Path utilities
    fs::path findHw3Img(const fs::path& inputPath);

    fs::path locateImageFolder(int argc, char* argv[]);

    // RAW image I/O
    GrayImage readRawGray(const fs::path& filePath, int width, int height);

    void writeRawGray(const fs::path& filePath, const GrayImage& image);

    // Histogram calculation
    Histogram calculateHistogram(const GrayImage& image);

    // Image quality measurement
    double calculatePSNR(const GrayImage& original, const GrayImage& processed);

    // PNG output
    void saveGrayPng(const fs::path& filePath, const GrayImage& image);

    void saveHistogramPng(const fs::path& filePath, const Histogram& histogram, uint32_t maxCount);

} // namespace hw3