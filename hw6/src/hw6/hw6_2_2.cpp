#include "hw6/hw6_2_2.hpp"
#include <iostream>
#include <sstream>

namespace hw6 {
// Q2-2: crossing number on every skeleton pixel. CN = 1 -> ridge ending, CN = 3 -> bifurcation.
// The result image marks endings in red and bifurcations in blue (5x5 hollow squares).
void runHw6_2_2(const Paths& paths) {
    const fs::path dir = paths.output / "hw6_2_2";
    const Image skeleton = zhangSuenThinning(invert(readRaw(paths.input / "fingerprint360x480.raw", 360, 480)));
    const auto minutiae = detectMinutiae(skeleton);

    // Light-gray skeleton on white so the coloured markers stand out.
    Image r(skeleton.width, skeleton.height, 255), g = r, b = r;
    for (std::size_t i = 0; i < skeleton.pixels.size(); ++i)
        if (skeleton.pixels[i] > 127) r.pixels[i] = g.pixels[i] = b.pixels[i] = 120;
    int endings = 0, bifurcations = 0;
    std::ostringstream csv;
    csv << "x,y,cn,type\n";
    for (const Minutia& m : minutiae) {
        const bool ending = m.cn == 1;
        (ending ? endings : bifurcations)++;
        csv << m.x << ',' << m.y << ',' << m.cn << ',' << (ending ? "ending" : "bifurcation") << '\n';
        for (int dy = -2; dy <= 2; ++dy)
            for (int dx = -2; dx <= 2; ++dx) {
                if (std::abs(dx) != 2 && std::abs(dy) != 2) continue;          // hollow square
                const int x = m.x + dx, y = m.y + dy;
                if (x < 0 || y < 0 || x >= skeleton.width || y >= skeleton.height) continue;
                r.at(x, y) = ending ? 230 : 0;
                g.at(x, y) = ending ? 0 : 90;
                b.at(x, y) = ending ? 0 : 230;
            }
    }
    saveRgbPng(dir / "fingerprint360x480_minutiae.png", r, g, b);
    writeText(dir / "minutiae.csv", csv.str());
    std::ostringstream summary;
    summary << "ridge_endings(CN=1)," << endings << "\nbifurcations(CN=3)," << bifurcations << '\n';
    writeText(dir / "count.csv", summary.str());
    std::cout << summary.str();
}
} // namespace hw6
