#include "hw4/common.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace hw4 {
namespace {
void checkSize(int size) {
    if (size <= 0 || size % 2 == 0) throw std::invalid_argument("Kernel must have positive odd size");
}
}
Image convolve(const Image& image, const std::vector<double>& kernel, int size) {
    checkSize(size);
    if (!image.valid() || kernel.size() != static_cast<std::size_t>(size) * size)
        throw std::invalid_argument("Invalid convolution input");
    Image result(image.width, image.height);
    const int radius = size / 2;
    for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x) {
        double sum = 0;
        for (int ky = 0; ky < size; ++ky) for (int kx = 0; kx < size; ++kx) {
            const int sx = std::clamp(x + kx - radius, 0, image.width - 1);
            const int sy = std::clamp(y + ky - radius, 0, image.height - 1);
            // True convolution (kernel reversal); symmetric smoothing kernels are unaffected.
            sum += image.at(sx, sy) * kernel[(size - 1 - ky) * size + size - 1 - kx];
        }
        result.at(x, y) = sum;
    }
    return result;
}
Image meanFilter(const Image& image, int size) {
    checkSize(size);
    return convolve(image, std::vector<double>(size * size, 1.0 / (size * size)), size);
}
Image gaussianFilter(const Image& image, int size, double sigma) {
    checkSize(size);
    if (!(sigma > 0) || !std::isfinite(sigma)) throw std::invalid_argument("Invalid sigma");
    std::vector<double> kernel(size * size);
    double sum = 0;
    for (int y = 0; y < size; ++y) for (int x = 0; x < size; ++x) {
        const double dx = x - size / 2, dy = y - size / 2;
        sum += kernel[y * size + x] = std::exp(-(dx * dx + dy * dy) / (2 * sigma * sigma));
    }
    for (double& p : kernel) p /= sum;
    return convolve(image, kernel, size);
}
Image medianFilter(const Image& image, int size) {
    checkSize(size);
    if (!image.valid()) throw std::invalid_argument("Invalid image");
    Image result(image.width, image.height);
    std::vector<double> window(size * size);
    for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x) {
        int i = 0;
        for (int dy = -size / 2; dy <= size / 2; ++dy)
            for (int dx = -size / 2; dx <= size / 2; ++dx)
                window[i++] = image.at(std::clamp(x + dx, 0, image.width - 1),
                                       std::clamp(y + dy, 0, image.height - 1));
        auto middle = window.begin() + window.size() / 2;
        std::nth_element(window.begin(), middle, window.end());
        result.at(x, y) = *middle;
    }
    return result;
}
} // namespace hw4
