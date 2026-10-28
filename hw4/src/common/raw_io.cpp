#include "hw4/common.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace hw4 {
Image::Image(int w, int h, double value) : width(w), height(h) {
    if (w <= 0 || h <= 0 || static_cast<std::size_t>(w) >
        std::numeric_limits<std::size_t>::max() / static_cast<std::size_t>(h))
        throw std::invalid_argument("Invalid image dimensions");
    pixels.assign(static_cast<std::size_t>(w) * h, value);
}
bool Image::valid() const {
    return width > 0 && height > 0 && pixels.size() == static_cast<std::size_t>(width) * height;
}
double& Image::at(int x, int y) { return pixels[static_cast<std::size_t>(y) * width + x]; }
double Image::at(int x, int y) const { return pixels[static_cast<std::size_t>(y) * width + x]; }
Image readRaw(const fs::path& file, int width, int height) {
    Image image(width, height);
    if (fs::file_size(file) != image.pixels.size())
        throw std::runtime_error("Incorrect 8-bit RAW size: " + file.string());
    std::ifstream stream(file, std::ios::binary);
    std::vector<unsigned char> bytes(image.pixels.size());
    if (!stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("Cannot read RAW: " + file.string());
    std::copy(bytes.begin(), bytes.end(), image.pixels.begin());
    return image;
}
Image toByteImage(const Image& image) {
    if (!image.valid()) throw std::invalid_argument("Invalid image");
    Image result = image;
    for (double& p : result.pixels) {
        if (!std::isfinite(p)) throw std::runtime_error("Non-finite image value");
        p = std::round(std::clamp(p, 0.0, 255.0));
    }
    return result;
}
void writeText(const fs::path& path, const std::string& text) {
    if (!path.parent_path().empty()) fs::create_directories(path.parent_path());
    std::ofstream stream(path);
    stream << text;
    stream.close();
    if (!stream) throw std::runtime_error("Cannot write: " + path.string());
}
} // namespace hw4
