#include "hw5/hw5_3_4.hpp"
#include <iostream>

namespace hw5 {
// Q3-4: pHash / aHash Hamming distances between lena256 and its three JPEG reconstructions.
void runHw5_3_4(const Paths& paths) {
    const Image lena = readRaw(paths.input / "lena256.raw", 256, 256);
    std::vector<NamedImage> images{{"lena256", lena}};
    images.push_back({"lena256_jpeg_QF20", jpegSimulate(lena, qualityTable(baseQuantTable(), 20)).reconstructed});
    images.push_back({"lena256_jpeg_Q", jpegSimulate(lena, baseQuantTable()).reconstructed});
    images.push_back({"lena256_jpeg_QF80", jpegSimulate(lena, qualityTable(baseQuantTable(), 80)).reconstructed});
    const std::string csv = hashReport(images, paths.output / "hw5_3_4");
    writeText(paths.output / "hw5_3_4" / "jpeg_hash.csv", csv);
    std::cout << csv;
}
} // namespace hw5
