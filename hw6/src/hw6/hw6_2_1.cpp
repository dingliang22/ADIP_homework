#include "hw6/hw6_2_1.hpp"
#include <iostream>

namespace hw6 {
// Q2-1: Zhang-Suen thinning, applied directly (no preprocessing). Ridges are black, so the
// image is inverted to make ridges the foreground; the skeleton is saved as black on white.
void runHw6_2_1(const Paths& paths) {
    const fs::path dir = paths.output / "hw6_2_1";
    const Image ridges = invert(readRaw(paths.input / "fingerprint360x480.raw", 360, 480));
    int iterations = 0;
    const Image skeleton = zhangSuenThinning(ridges, &iterations);
    saveImage(dir / "fingerprint360x480_thinned", invert(skeleton));
    std::cout << "thinning iterations: " << iterations << '\n';
}
} // namespace hw6
