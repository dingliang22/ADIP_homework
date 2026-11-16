#include "hw5/hw5_3_3.hpp"
#include <iostream>
#include <sstream>

namespace hw5 {
// Q3-3: MSE / PSNR of the three reconstructions, plus zoomed crops and error maps for artifacts.
void runHw5_3_3(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_3_3";
    const Image lena = readRaw(paths.input / "lena256.raw", 256, 256);
    struct Case { std::string name; QTable table; };
    const std::vector<Case> cases{{"QF20", qualityTable(baseQuantTable(), 20)},
                                  {"Q", baseQuantTable()},
                                  {"QF80", qualityTable(baseQuantTable(), 80)}};
    // Crop: face / hat-brim region (64x64 from (104,88)), shown at 4x with nearest neighbour.
    constexpr int cropX = 104, cropY = 88, cropSize = 64, zoom = 4;
    saveImage(dir / "lena256_original_crop_x4", zoomNearest(lena, cropX, cropY, cropSize, cropSize, zoom));
    std::ostringstream csv;
    csv << "case,MSE,PSNR_dB,nonzero_coefficients\n";
    for (const Case& c : cases) {
        const JpegResult r = jpegSimulate(lena, c.table);
        saveImage(dir / ("lena256_jpeg_" + c.name + "_crop_x4"),
                  zoomNearest(r.reconstructed, cropX, cropY, cropSize, cropSize, zoom));
        Image error(lena.width, lena.height);
        for (std::size_t i = 0; i < error.pixels.size(); ++i)
            error.pixels[i] = 128.0 + 4.0 * (r.reconstructed.pixels[i] - lena.pixels[i]);   // x4 gain
        saveImage(dir / ("lena256_jpeg_" + c.name + "_error_x4"), error);
        csv << c.name << ',' << mse(lena, r.reconstructed) << ',' << psnr(lena, r.reconstructed) << ','
            << r.nonZeroCoefficients << '\n';
    }
    writeText(dir / "mse_psnr.csv", csv.str());
    std::cout << csv.str();
}
} // namespace hw5
