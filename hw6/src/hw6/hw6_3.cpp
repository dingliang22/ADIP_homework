#include "hw6/hw6_3.hpp"
#include <algorithm>
#include <iostream>

namespace hw6 {
// Q3: top-hat (bright details smaller than the SE) and bottom-hat (dark details) with disk r=10;
// enhanced = f + TopHat - BottomHat, clipped to [0,255].
void runHw6_3(const Paths& paths) {
    const fs::path dir = paths.output / "hw6_3";
    const Image f = readRaw(paths.input / "wake1274x716.raw", 1274, 716);
    const auto se = diskSE(10);   // r=5 only sharpens fine texture; r=15 adds halos around ships
    const Image topHat = whiteTopHat(f, se);
    const Image bottomHat = blackTopHat(f, se);
    Image enhanced = f;
    for (std::size_t i = 0; i < f.pixels.size(); ++i)
        enhanced.pixels[i] = std::clamp(f.pixels[i] + topHat.pixels[i] - bottomHat.pixels[i], 0.0, 255.0);
    // Hat images are small positive values; stretched to [0,255] for display.
    saveImage(dir / "wake1274x716_tophat", normalizeMinMax(topHat));
    saveImage(dir / "wake1274x716_bottomhat", normalizeMinMax(bottomHat));
    saveImage(dir / "wake1274x716_enhanced", enhanced);
    const auto [tlo, thi] = std::minmax_element(topHat.pixels.begin(), topHat.pixels.end());
    const auto [blo, bhi] = std::minmax_element(bottomHat.pixels.begin(), bottomHat.pixels.end());
    std::cout << "tophat range " << *tlo << ".." << *thi << ", bottomhat range " << *blo << ".." << *bhi << '\n';
}
} // namespace hw6
