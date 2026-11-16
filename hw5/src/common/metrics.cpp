#include "hw5/common.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace hw5 {
namespace {
void checkPair(const Image& a, const Image& b) {
    if (!a.valid() || !b.valid() || a.width != b.width || a.height != b.height)
        throw std::invalid_argument("Image dimensions differ");
}
} // namespace
double mse(const Image& reference, const Image& image) {
    checkPair(reference, image);
    double sum = 0;
    for (std::size_t i = 0; i < image.pixels.size(); ++i) {
        const double d = reference.pixels[i] - image.pixels[i];
        sum += d * d;
    }
    return sum / static_cast<double>(image.pixels.size());
}
double psnr(const Image& reference, const Image& image) {
    const double e = mse(reference, image);
    if (e == 0) return std::numeric_limits<double>::infinity();
    return 10 * std::log10(255.0 * 255.0 / e);
}
double maxAbsDifference(const Image& a, const Image& b) {
    checkPair(a, b);
    double m = 0;
    for (std::size_t i = 0; i < a.pixels.size(); ++i) m = std::max(m, std::abs(a.pixels[i] - b.pixels[i]));
    return m;
}
} // namespace hw5
