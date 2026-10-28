#include "hw4/common.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace hw4 {
Image nearestResize(const Image& image, int width, int height) {
    if (!image.valid()) throw std::invalid_argument("Invalid resize input");
    Image result(width, height);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        const int sx = std::min(image.width - 1, static_cast<int>((x + 0.5) * image.width / width));
        const int sy = std::min(image.height - 1, static_cast<int>((y + 0.5) * image.height / height));
        result.at(x, y) = image.at(sx, sy);
    }
    return result;
}
Image bilinearResize(const Image& image, int width, int height) {
    if (!image.valid()) throw std::invalid_argument("Invalid resize input");
    Image result(width, height);
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        const double sx = std::clamp((x + 0.5) * image.width / width - 0.5, 0.0, image.width - 1.0);
        const double sy = std::clamp((y + 0.5) * image.height / height - 0.5, 0.0, image.height - 1.0);
        const int x0 = static_cast<int>(sx), y0 = static_cast<int>(sy);
        const int x1 = std::min(x0 + 1, image.width - 1), y1 = std::min(y0 + 1, image.height - 1);
        const double fx = sx - x0, fy = sy - y0;
        result.at(x, y) = (1 - fy) * ((1 - fx) * image.at(x0, y0) + fx * image.at(x1, y0)) +
                         fy * ((1 - fx) * image.at(x0, y1) + fx * image.at(x1, y1));
    }
    return result;
}
Image decimate2(const Image& image) {
    if (!image.valid() || image.width % 2 || image.height % 2)
        throw std::invalid_argument("Downsampling requires even dimensions");
    // Sample at the center of each 2x2 cell, consistent with half-pixel upsampling.
    Image result(image.width / 2, image.height / 2);
    for (int y = 0; y < result.height; ++y) for (int x = 0; x < result.width; ++x)
        result.at(x, y) = (image.at(2*x, 2*y) + image.at(2*x+1, 2*y) +
                          image.at(2*x, 2*y+1) + image.at(2*x+1, 2*y+1)) * 0.25;
    return result;
}
Images ibpSnapshots(const Image& lowResolution) {
    Image current = bilinearResize(lowResolution, lowResolution.width * 2, lowResolution.height * 2);
    Images results;
    for (int iteration = 1; iteration <= 10; ++iteration) {
        const Image simulated = decimate2(gaussianFilter(current, 5, 0.8));
        Image error = lowResolution;
        for (std::size_t i = 0; i < error.pixels.size(); ++i)
            error.pixels[i] -= simulated.pixels[i];
        const Image correction = gaussianFilter(bilinearResize(error, current.width, current.height), 5, 0.8);
        for (std::size_t i = 0; i < current.pixels.size(); ++i)
            current.pixels[i] = std::clamp(current.pixels[i] + 0.8 * correction.pixels[i], 0.0, 255.0);
        if (iteration == 1 || iteration == 3 || iteration == 5 || iteration == 10)
            results.emplace("ibp_" + std::to_string(iteration), current);
    }
    return results;
}
} // namespace hw4
