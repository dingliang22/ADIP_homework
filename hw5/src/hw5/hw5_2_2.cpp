#include "hw5/hw5_2_2.hpp"
#include <iostream>
#include <sstream>

namespace hw5 {
// Q2-2: pHash (32x32 area resize, 32x32 cv::dct, 8x8 low band, mean without DC) + Hamming distance.
void runHw5_2_2(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_2_2";
    const auto images = loadLenaSet(paths);
    const std::uint64_t query = pHash(images.front().image);
    std::ostringstream csv;
    csv << "image,pHash_hex,hamming_to_lena256,similarity_percent\n";
    for (const NamedImage& item : images) {
        Image block;
        const std::uint64_t hash = pHash(item.image, &block);
        saveImage(dir / item.name, item.image);
        saveImage(dir / (item.name + "_resized32_x8"), zoomNearest(areaResize(item.image, 32, 32), 0, 0, 32, 32, 8));
        saveImage(dir / (item.name + "_pHash_bits"), hashBitmap(hash));
        const int distance = hamming(query, hash);
        csv << item.name << ',' << hashHex(hash) << ',' << distance << ',' << 100.0 * (64 - distance) / 64 << '\n';
    }
    writeText(dir / "phash.csv", csv.str());
    std::cout << csv.str();
}
} // namespace hw5
