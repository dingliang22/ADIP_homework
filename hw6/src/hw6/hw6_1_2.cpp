#include "hw6/hw6_1_2.hpp"
#include <iostream>

namespace hw6 {
namespace {
// Replace pixels where `detail` > threshold (mask dilated by `grow`) with `fallback`.
Image replaceWhere(const Image& base, const Image& detail, double threshold, const Image& fallback,
                   int grow, int* count) {
    Image mask(base.width, base.height);
    for (std::size_t i = 0; i < mask.pixels.size(); ++i) mask.pixels[i] = detail.pixels[i] > threshold ? 255 : 0;
    if (grow > 0) mask = dilate(mask, squareSE(grow));
    Image result = base;
    int n = 0;
    for (std::size_t i = 0; i < result.pixels.size(); ++i)
        if (mask.pixels[i] > 0) { result.pixels[i] = fallback.pixels[i]; ++n; }
    if (count) *count = n;
    return result;
}
} // namespace

// Q1-2: grayscale cell restoration.
// Noise = small, high-contrast bright specks / scratches and dark specks; cell texture (granules,
// membranes) has lower contrast. Top-hat / bottom-hat (disk r=4) isolate small bright / dark
// details; only pixels whose response exceeds 40 (mask grown by 1 px to cover halos) are replaced
// by the opened / closed value, everything else keeps its original gray level.
// A larger disk removes a few bigger blobs too but flattens nuclear texture and membranes.
void runHw6_1_2(const Paths& paths) {
    const fs::path dir = paths.output / "hw6_1_2";
    const Image f = readRaw(paths.input / "cell1200x900.raw", 1200, 900);
    const auto se = diskSE(4);
    constexpr double threshold = 40;
    int nBright = 0, nDark = 0;

    const Image topHat = whiteTopHat(f, se);
    const Image noBright = replaceWhere(f, topHat, threshold, opening(f, se), 1, &nBright);
    const Image closed = closing(noBright, se);
    Image bottomHat = closed;
    for (std::size_t i = 0; i < bottomHat.pixels.size(); ++i) bottomHat.pixels[i] -= noBright.pixels[i];
    const Image restored = replaceWhere(noBright, bottomHat, threshold, closed, 1, &nDark);

    saveImage(dir / "cell1200x900_restored", restored);
    std::cout << "replaced bright " << nBright << ", dark " << nDark << " pixels\n";
}
} // namespace hw6
