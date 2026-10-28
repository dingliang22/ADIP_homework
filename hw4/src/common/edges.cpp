#include "hw4/common.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace hw4 {
namespace {
Image absolute(const Image& image) {
    Image result = image;
    for (double& p : result.pixels) p = std::abs(p);
    return result;
}
}
EdgeResult laplacian(const Image& image, bool eightNeighbors) {
    const std::vector<double> kernel = eightNeighbors
        ? std::vector<double>{1,1,1,1,-8,1,1,1,1}
        : std::vector<double>{0,1,0,1,-4,1,0,1,0};
    Image response = convolve(image, kernel, 3);
    return {eightNeighbors ? "laplacian8" : "laplacian4", response, absolute(response)};
}
EdgeResult sobel(const Image& image, int direction) {
    // Kernels are reversed here because convolve performs mathematical convolution.
    const Image gx = convolve(image, {1,0,-1,2,0,-2,1,0,-1}, 3);
    const Image gy = convolve(image, {1,2,1,0,0,0,-1,-2,-1}, 3);
    if (direction == 0) return {"sobel0", gx, absolute(gx)};
    if (direction == 90) return {"sobel90", gy, absolute(gy)};
    if (direction != -1) throw std::invalid_argument("Invalid Sobel direction");
    Image magnitude(image.width, image.height);
    for (std::size_t i = 0; i < magnitude.pixels.size(); ++i)
        magnitude.pixels[i] = std::hypot(gx.pixels[i], gy.pixels[i]);
    return {"sobel_combined", magnitude, magnitude};
}
EdgeResult logEdges(const Image& image) {
    const Image response = laplacian(gaussianFilter(image, 5, 1.0), false).response;
    Image strength(image.width, image.height);
    // Mark the smaller-magnitude endpoint of each 8-neighbor sign change.
    // Exact-zero pixels are marked only when opposite neighbors have opposite signs.
    for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x) {
        const double center = response.at(x, y);
        double contrast = 0;
        for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            const int nx = x + dx, ny = y + dy;
            if (nx < 0 || nx >= image.width || ny < 0 || ny >= image.height) continue;
            const double other = response.at(nx, ny);
            if (center * other < 0 && std::abs(center) <= std::abs(other))
                contrast = std::max(contrast, std::abs(center - other));
            const int px = x - dx, py = y - dy;
            if (center == 0 && px >= 0 && px < image.width && py >= 0 && py < image.height) {
                const double opposite = response.at(px, py);
                if (other * opposite < 0) contrast = std::max(contrast, std::abs(other - opposite));
            }
        }
        strength.at(x, y) = contrast;
    }
    return {"log", response, strength};
}
double percentile(const Image& image, double fraction) {
    if (!image.valid() || fraction < 0 || fraction > 1) throw std::invalid_argument("Invalid percentile");
    std::vector<double> values = image.pixels;
    const auto index = static_cast<std::size_t>(fraction * (values.size() - 1));
    std::nth_element(values.begin(), values.begin() + index, values.end());
    return values[index];
}
Image signedPreview(const Image& response) {
    const double scale = percentile(absolute(response), 0.99);
    Image result(response.width, response.height, 128);
    if (scale > 0) for (std::size_t i = 0; i < result.pixels.size(); ++i)
        result.pixels[i] = 128.0 + 127.0 * std::clamp(response.pixels[i] / scale, -1.0, 1.0);
    return result;
}
Image magnitudePreview(const Image& strength) {
    const double scale = percentile(strength, 0.99);
    Image result(strength.width, strength.height);
    if (scale > 0) for (std::size_t i = 0; i < result.pixels.size(); ++i)
        result.pixels[i] = 255.0 * std::clamp(strength.pixels[i] / scale, 0.0, 1.0);
    return result;
}
Image binarize(const Image& strength, double threshold) {
    if (!strength.valid() || !std::isfinite(threshold) || threshold < 0)
        throw std::invalid_argument("Invalid binary threshold");
    Image result = strength;
    for (double& p : result.pixels) p = p > threshold ? 255.0 : 0.0;
    return result;
}
} // namespace hw4
