#include "hw6/common.hpp"
#include <algorithm>
#include <deque>
#include <limits>
#include <stdexcept>

namespace hw6 {
StructuringElement squareSE(int radius) {
    StructuringElement se;
    for (int dy = -radius; dy <= radius; ++dy)
        for (int dx = -radius; dx <= radius; ++dx) se.push_back({dx, dy});
    return se;
}
StructuringElement diskSE(int radius) {
    StructuringElement se;
    for (int dy = -radius; dy <= radius; ++dy)
        for (int dx = -radius; dx <= radius; ++dx)
            if (dx * dx + dy * dy <= radius * radius) se.push_back({dx, dy});
    return se;
}

namespace {
// Flat grayscale erosion (min) / dilation (max). Dilation uses the reflected SE so that
// opening/closing are correct for asymmetric elements too (all SEs here are symmetric).
template <bool IsDilation> Image rankFilter(const Image& image, const StructuringElement& se) {
    if (!image.valid() || se.empty()) throw std::invalid_argument("Invalid morphology input");
    Image result(image.width, image.height);
    for (int y = 0; y < image.height; ++y)
        for (int x = 0; x < image.width; ++x) {
            double value = IsDilation ? -std::numeric_limits<double>::infinity()
                                      : std::numeric_limits<double>::infinity();
            for (const Offset& o : se) {
                const int sx = IsDilation ? x - o.dx : x + o.dx;
                const int sy = IsDilation ? y - o.dy : y + o.dy;
                if (sx < 0 || sy < 0 || sx >= image.width || sy >= image.height) continue;
                const double p = image.at(sx, sy);
                value = IsDilation ? std::max(value, p) : std::min(value, p);
            }
            result.at(x, y) = value;
        }
    return result;
}
} // namespace

Image erode(const Image& image, const StructuringElement& se) { return rankFilter<false>(image, se); }
Image dilate(const Image& image, const StructuringElement& se) { return rankFilter<true>(image, se); }
Image opening(const Image& image, const StructuringElement& se) { return dilate(erode(image, se), se); }
Image closing(const Image& image, const StructuringElement& se) { return erode(dilate(image, se), se); }

Image whiteTopHat(const Image& image, const StructuringElement& se) {
    Image result = opening(image, se);
    for (std::size_t i = 0; i < result.pixels.size(); ++i) result.pixels[i] = image.pixels[i] - result.pixels[i];
    return result;
}
Image blackTopHat(const Image& image, const StructuringElement& se) {
    Image result = closing(image, se);
    for (std::size_t i = 0; i < result.pixels.size(); ++i) result.pixels[i] -= image.pixels[i];
    return result;
}

// L. Vincent, "Morphological grayscale reconstruction in image analysis", 1993 (hybrid algorithm).
Image reconstructByDilation(const Image& marker, const Image& mask) {
    const int w = mask.width, h = mask.height;
    Image j(w, h);
    for (std::size_t i = 0; i < j.pixels.size(); ++i) j.pixels[i] = std::min(marker.pixels[i], mask.pixels[i]);
    const auto inside = [&](int x, int y) { return x >= 0 && y >= 0 && x < w && y < h; };
    // Raster scan: N+(p) = already-visited neighbours (up row + left).
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            double m = j.at(x, y);
            for (const auto [dx, dy] : {std::pair{-1, -1}, {0, -1}, {1, -1}, {-1, 0}})
                if (inside(x + dx, y + dy)) m = std::max(m, j.at(x + dx, y + dy));
            j.at(x, y) = std::min(m, mask.at(x, y));
        }
    // Anti-raster scan, queueing pixels that can still propagate.
    std::deque<std::pair<int, int>> fifo;
    constexpr std::pair<int, int> back[] = {{1, 1}, {0, 1}, {-1, 1}, {1, 0}};
    for (int y = h - 1; y >= 0; --y)
        for (int x = w - 1; x >= 0; --x) {
            double m = j.at(x, y);
            for (const auto [dx, dy] : back)
                if (inside(x + dx, y + dy)) m = std::max(m, j.at(x + dx, y + dy));
            j.at(x, y) = std::min(m, mask.at(x, y));
            for (const auto [dx, dy] : back) {
                const int qx = x + dx, qy = y + dy;
                if (inside(qx, qy) && j.at(qx, qy) < j.at(x, y) && j.at(qx, qy) < mask.at(qx, qy)) {
                    fifo.emplace_back(x, y);
                    break;
                }
            }
        }
    // Propagation.
    while (!fifo.empty()) {
        const auto [x, y] = fifo.front();
        fifo.pop_front();
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx) {
                const int qx = x + dx, qy = y + dy;
                if ((!dx && !dy) || !inside(qx, qy)) continue;
                if (j.at(qx, qy) < j.at(x, y) && j.at(qx, qy) != mask.at(qx, qy)) {
                    j.at(qx, qy) = std::min(j.at(x, y), mask.at(qx, qy));
                    fifo.emplace_back(qx, qy);
                }
            }
    }
    return j;
}

Image openingByReconstruction(const Image& image, const StructuringElement& se) {
    return reconstructByDilation(erode(image, se), image);
}

// Area opening on a binary image: removes 8-connected foreground components smaller than minArea.
Image removeSmallComponents(const Image& binary, int minArea) {
    const int w = binary.width, h = binary.height;
    Image result = binary;
    std::vector<char> seen(binary.pixels.size(), 0);
    std::vector<int> stack, component;
    for (int start = 0; start < w * h; ++start) {
        if (seen[start] || binary.pixels[start] <= 127) continue;
        stack = {start};
        component.clear();
        seen[start] = 1;
        while (!stack.empty()) {
            const int p = stack.back();
            stack.pop_back();
            component.push_back(p);
            const int px = p % w, py = p / w;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    const int qx = px + dx, qy = py + dy;
                    if (qx < 0 || qy < 0 || qx >= w || qy >= h) continue;
                    const int q = qy * w + qx;
                    if (!seen[q] && binary.pixels[q] > 127) { seen[q] = 1; stack.push_back(q); }
                }
        }
        if (static_cast<int>(component.size()) < minArea)
            for (int p : component) result.pixels[p] = 0;
    }
    return result;
}
} // namespace hw6
