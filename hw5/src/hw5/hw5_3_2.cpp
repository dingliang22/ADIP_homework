#include "hw5/hw5_3_2.hpp"
#include <iostream>

namespace hw5 {
// Q3-2: quality-scaled tables (QF = 20, 80) and their reconstructions.
void runHw5_3_2(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_3_2";
    const Image lena = readRaw(paths.input / "lena256.raw", 256, 256);
    std::string text;
    for (int qf : {20, 80}) {
        const QTable table = qualityTable(baseQuantTable(), qf);
        const JpegResult result = jpegSimulate(lena, table);
        saveImage(dir / ("lena256_jpeg_QF" + std::to_string(qf)), result.reconstructed);
        text += "QF=" + std::to_string(qf) + " (S=" + (qf < 50 ? std::to_string(5000 / qf) : std::to_string(200 - 2 * qf)) +
                ")\n" + formatTable(table) + "nonzero_coefficients," + std::to_string(result.nonZeroCoefficients) +
                "/65536\n\n";
    }
    writeText(dir / "quant_tables.txt", text);
    std::cout << text;
}
} // namespace hw5
