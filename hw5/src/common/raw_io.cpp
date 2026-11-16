#include "hw5/common.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace hw5 {
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

Spectrum::Spectrum(int w, int h) : width(w), height(h), data(static_cast<std::size_t>(w) * h) {}
std::complex<double>& Spectrum::at(int u, int v) { return data[static_cast<std::size_t>(v) * width + u]; }
const std::complex<double>& Spectrum::at(int u, int v) const {
    return data[static_cast<std::size_t>(v) * width + u];
}

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
Image normalizeMinMax(const Image& image) {
    const auto [lo, hi] = std::minmax_element(image.pixels.begin(), image.pixels.end());
    Image result = image;
    const double range = *hi - *lo;
    for (double& p : result.pixels) p = range > 0 ? (p - *lo) * 255.0 / range : 0.0;
    return result;
}
Image normalizePercentile(const Image& image, double low, double high) {
    std::vector<double> sorted = image.pixels;
    std::sort(sorted.begin(), sorted.end());
    const auto pick = [&](double f) { return sorted[static_cast<std::size_t>(f * (sorted.size() - 1))]; };
    const double lo = pick(low), hi = pick(high);
    Image result = image;
    for (double& p : result.pixels) p = hi > lo ? std::clamp((p - lo) * 255.0 / (hi - lo), 0.0, 255.0) : 128.0;
    return result;
}
Image zoomNearest(const Image& image, int x0, int y0, int w, int h, int factor) {
    Image result(w * factor, h * factor);
    for (int y = 0; y < result.height; ++y)
        for (int x = 0; x < result.width; ++x)
            result.at(x, y) = image.at(x0 + x / factor, y0 + y / factor);
    return result;
}
} // namespace hw5
