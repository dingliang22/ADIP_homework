#include "hw3/common.hpp"

#include <fstream>
#include <stdexcept>

namespace hw3 {

    GrayImage readRawGray(const fs::path& filePath, int width, int height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Invalid image dimensions.");
        }

        const size_t totalPixels = static_cast<size_t>(width) * height;

        std::ifstream file(filePath, std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open RAW file: " + filePath.string());
        }

        // Check expected RAW file size.
        file.seekg(0, std::ios::end);

        const auto actualSize = file.tellg();

        if (actualSize != static_cast<std::streamoff>(totalPixels)) {
            throw std::runtime_error("Incorrect RAW file size: " + filePath.string());
        }

        file.seekg(0, std::ios::beg);

        GrayImage image;

        image.width = width;
        image.height = height;
        image.pixels.resize(totalPixels);

        file.read(reinterpret_cast<char*>(image.pixels.data()),
                  static_cast<std::streamsize>(totalPixels));

        if (!file) {
            throw std::runtime_error("Failed to read RAW file: " + filePath.string());
        }

        return image;
    }

    void writeRawGray(const fs::path& filePath, const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for RAW output.");
        }

        std::ofstream file(filePath, std::ios::binary | std::ios::trunc);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot create RAW file: " + filePath.string());
        }

        file.write(reinterpret_cast<const char*>(image.pixels.data()),
                   static_cast<std::streamsize>(image.pixels.size()));

        if (!file) {
            throw std::runtime_error("Failed to write RAW file: " + filePath.string());
        }
    }

} // namespace hw3