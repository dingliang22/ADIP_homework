#include "hw4/hw4_2_1.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace hw4 {
    void runHw4_2_1(const Paths& paths) {
        const Image low = readRaw(paths.input / "lena256.raw", 256, 256);
        const Image reference = readRaw(paths.input / "lena512.raw", 512, 512);
        const Images results = ibpSnapshots(low);
        saveImages(paths.output / "hw4_2_1", results);
        std::ostringstream table;
        table << std::fixed << std::setprecision(8) << "iteration,psnr_db\n";
        for (int iteration : {1, 3, 5, 10}) {
            table << iteration << ','
                  << psnr(reference, toByteImage(results.at("ibp_" + std::to_string(iteration))))
                  << '\n';
        }
        writeText(paths.output / "hw4_2_1" / "psnr.csv", table.str());
        std::cout << table.str();
    }
} // namespace hw4
