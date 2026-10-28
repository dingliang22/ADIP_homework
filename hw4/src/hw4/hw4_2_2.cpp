#include "hw4/hw4_2_2.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace hw4 {
    void runHw4_2_2(const Paths& paths) {
        const Image low = readRaw(paths.input / "lena256.raw", 256, 256);
        const Image reference = readRaw(paths.input / "lena512.raw", 512, 512);
        Images results{{"nearest", nearestResize(low, 512, 512)},
                       {"bilinear", bilinearResize(low, 512, 512)}};
        saveImages(paths.output / "hw4_2_2", results);
        results.merge(ibpSnapshots(low));
        std::ostringstream table;
        table << std::fixed << std::setprecision(8) << "method,psnr_db\n";
        for (const std::string name :
             {"nearest", "bilinear", "ibp_1", "ibp_3", "ibp_5", "ibp_10"}) {
            table << name << ',' << psnr(reference, toByteImage(results.at(name))) << '\n';
        }
        writeText(paths.output / "hw4_2_2" / "comparison.csv", table.str());
        std::cout << table.str();
    }
} // namespace hw4
