#include "hw6/hw6_1_1.hpp"
#include <iostream>

namespace hw6 {
// Q1-1: binary text restoration. Text (black) is the foreground, so the image is inverted first.
//   1) Area opening (< 3 px): remove the finest dust so it is not merged into blobs by step 2.
//   2) Closing 3x3: fill pin-holes inside strokes and reconnect 1-2 px breaks.
//   3) Closing with a vertical 1x5 line: most cracks cut across vertical stems, so this bridges
//      them without merging neighbouring letters horizontally.
//   4) Area opening (< 30 px): remove the remaining isolated specks; reconnected letters, even in
//      the thin bottom row, are larger than this.
void runHw6_1_1(const Paths& paths) {
    const fs::path dir = paths.output / "hw6_1_1";
    const Image text = invert(readRaw(paths.input / "text1480x460_binary.raw", 1480, 460));
    StructuringElement verticalLine;
    for (int dy = -2; dy <= 2; ++dy) verticalLine.push_back({0, dy});

    const Image step1 = removeSmallComponents(text, 3);
    const Image step2 = closing(step1, squareSE(1));
    const Image step3 = closing(step2, verticalLine);
    const Image restored = removeSmallComponents(step3, 30);

    saveImage(dir / "text1480x460_restored", invert(restored));
    std::cout << "saved " << (dir / "text1480x460_restored.png").string() << '\n';
}
} // namespace hw6
