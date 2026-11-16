#include "hw5/hw5_2_3.hpp"
#include <iostream>
#include <sstream>

namespace hw5 {
// Q2-3: aHash (8x8 area resize, bit = pixel >= mean) + Hamming distance.
void runHw5_2_3(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_2_3";
    const auto images = loadLenaSet(paths);
    const std::uint64_t query = aHash(images.front().image);
    std::ostringstream csv;
    csv << "image,aHash_hex,hamming_to_lena256,similarity_percent\n";
    for (const NamedImage& item : images) {
        Image small;
        const std::uint64_t hash = aHash(item.image, &small);
        saveImage(dir / (item.name + "_resized8_x32"), zoomNearest(small, 0, 0, 8, 8, 32));
        saveImage(dir / (item.name + "_aHash_bits"), hashBitmap(hash));
        const int distance = hamming(query, hash);
        csv << item.name << ',' << hashHex(hash) << ',' << distance << ',' << 100.0 * (64 - distance) / 64 << '\n';
    }
    writeText(dir / "ahash.csv", csv.str());
    std::cout << csv.str();
}
} // namespace hw5
