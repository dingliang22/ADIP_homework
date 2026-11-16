#include "hw5/hw5_3_1.hpp"
#include <iostream>

namespace hw5 {
// Q3-1: 8x8 block JPEG simulation (DCT + quantization + reconstruction) with the given table Q.
void runHw5_3_1(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_3_1";
    const Image lena = readRaw(paths.input / "lena256.raw", 256, 256);
    const JpegResult result = jpegSimulate(lena, baseQuantTable());
    saveImage(dir / "lena256_original", lena);
    saveImage(dir / "lena256_jpeg_Q", result.reconstructed);
    const std::string text = "table Q (given, equals QF=50):\n" + formatTable(baseQuantTable()) +
                             "nonzero_coefficients," + std::to_string(result.nonZeroCoefficients) + "/65536\n";
    writeText(dir / "result.txt", text);
    std::cout << text;
}
} // namespace hw5
