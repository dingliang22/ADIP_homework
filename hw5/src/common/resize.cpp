#include "hw5/common.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace hw5 {
// Box (area) resampling with fractional overlap weights; for 256->32 or 256->8 it reduces to an
// exact mean of each 8x8 or 32x32 block, which acts as an anti-aliasing low-pass before hashing.
Image areaResize(const Image& image, int width, int height) {
    if (!image.valid() || width <= 0 || height <= 0) throw std::invalid_argument("Invalid resize");
    const double sx = static_cast<double>(image.width) / width;
    const double sy = static_cast<double>(image.height) / height;
    Image result(width, height);
    for (int y = 0; y < height; ++y) {
        const double y0 = y * sy, y1 = (y + 1) * sy;
        for (int x = 0; x < width; ++x) {
            const double x0 = x * sx, x1 = (x + 1) * sx;
            double sum = 0, area = 0;
            for (int iy = static_cast<int>(std::floor(y0)); iy < std::min(image.height, static_cast<int>(std::ceil(y1))); ++iy) {
                const double wy = std::min(y1, iy + 1.0) - std::max(y0, static_cast<double>(iy));
                for (int ix = static_cast<int>(std::floor(x0)); ix < std::min(image.width, static_cast<int>(std::ceil(x1))); ++ix) {
                    const double wx = std::min(x1, ix + 1.0) - std::max(x0, static_cast<double>(ix));
                    sum += wx * wy * image.at(ix, iy);
                    area += wx * wy;
                }
            }
            result.at(x, y) = sum / area;
        }
    }
    return result;
}
} // namespace hw5
