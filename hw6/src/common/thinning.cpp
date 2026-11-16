#include "hw6/common.hpp"
#include <array>
#include <cstdlib>

namespace hw6 {
namespace {
// Clockwise 8-neighbourhood starting at north: P1=N, P2=NE, P3=E, P4=SE, P5=S, P6=SW, P7=W, P8=NW.
constexpr std::array<std::pair<int, int>, 8> kClockwise{
    {{0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}}};

std::array<int, 8> neighbours(const std::vector<unsigned char>& img, int w, int h, int x, int y) {
    std::array<int, 8> p{};
    for (int i = 0; i < 8; ++i) {
        const int qx = x + kClockwise[i].first, qy = y + kClockwise[i].second;
        p[i] = (qx >= 0 && qy >= 0 && qx < w && qy < h) ? img[qy * w + qx] : 0;   // outside = background
    }
    return p;
}
} // namespace

// Zhang & Suen (1984). Two sub-iterations; a foreground pixel is deleted when
//   2 <= B(P) <= 6, A(P) == 1 and
//   step 1: P1*P3*P5 == 0 and P3*P5*P7 == 0   (N*E*S, E*S*W)
//   step 2: P1*P3*P7 == 0 and P1*P5*P7 == 0   (N*E*W, N*S*W)
// where B = number of foreground neighbours, A = number of 0->1 transitions in clockwise order.
Image zhangSuenThinning(const Image& binary, int* iterations) {
    const int w = binary.width, h = binary.height;
    std::vector<unsigned char> img(binary.pixels.size());
    for (std::size_t i = 0; i < img.size(); ++i) img[i] = binary.pixels[i] > 127 ? 1 : 0;
    std::vector<int> toDelete;
    int passes = 0;
    bool changed = true;
    while (changed) {
        changed = false;
        ++passes;
        for (int step = 0; step < 2; ++step) {
            toDelete.clear();
            for (int y = 0; y < h; ++y)
                for (int x = 0; x < w; ++x) {
                    if (!img[y * w + x]) continue;
                    const auto p = neighbours(img, w, h, x, y);
                    int b = 0, a = 0;
                    for (int i = 0; i < 8; ++i) {
                        b += p[i];
                        if (!p[i] && p[(i + 1) % 8]) ++a;
                    }
                    if (b < 2 || b > 6 || a != 1) continue;
                    const bool c = step == 0 ? (p[0] * p[2] * p[4] == 0 && p[2] * p[4] * p[6] == 0)
                                             : (p[0] * p[2] * p[6] == 0 && p[0] * p[4] * p[6] == 0);
                    if (c) toDelete.push_back(y * w + x);
                }
            for (int idx : toDelete) img[idx] = 0;
            if (!toDelete.empty()) changed = true;
        }
    }
    if (iterations) *iterations = passes;
    Image result(w, h);
    for (std::size_t i = 0; i < img.size(); ++i) result.pixels[i] = img[i] ? 255.0 : 0.0;
    return result;
}

int crossingNumber(const Image& skeleton, int x, int y) {
    int sum = 0;
    int p[9];
    for (int i = 0; i < 8; ++i) {
        const int qx = x + kClockwise[i].first, qy = y + kClockwise[i].second;
        p[i] = (qx >= 0 && qy >= 0 && qx < skeleton.width && qy < skeleton.height && skeleton.at(qx, qy) > 127) ? 1 : 0;
    }
    p[8] = p[0];                                   // P9 = P1
    for (int i = 0; i < 8; ++i) sum += std::abs(p[i] - p[i + 1]);
    return sum / 2;
}

std::vector<Minutia> detectMinutiae(const Image& skeleton) {
    std::vector<Minutia> result;
    for (int y = 0; y < skeleton.height; ++y)
        for (int x = 0; x < skeleton.width; ++x) {
            if (skeleton.at(x, y) <= 127) continue;
            const int cn = crossingNumber(skeleton, x, y);
            if (cn == 1 || cn == 3) result.push_back({x, y, cn});
        }
    return result;
}
} // namespace hw6
