#include "hw4/hw4_1_4.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

namespace hw4 {
    void runHw4_1_4(const Paths& paths) {
        const Image reference = readRaw(paths.input / "town500.raw", 500, 500);
        const Image noisy = readRaw(paths.input / "town500_noisy.raw", 500, 500);
        Images results = denoiseSingles(noisy);
        results.merge(denoiseMedians(noisy));
        const Images cascades = denoiseCascades(noisy);
        results.insert(cascades.begin(), cascades.end());
        std::ostringstream table;
        table << std::fixed << std::setprecision(8) << "method,psnr_db\nnoisy_baseline,"
              << psnr(reference, noisy) << '\n';
        for (const auto& [name, image] : results) {
            table << name << ',' << psnr(reference, toByteImage(image)) << '\n';
        }
        writeText(paths.output / "hw4_1_4" / "psnr.csv", table.str());
        std::ostringstream order;
        order << std::fixed << std::setprecision(10)
              << "first,second,float_mae,float_different_pixels,byte_mae,byte_different_pixels\n";
        for (const auto& pair : {std::pair{"median_then_gaussian", "gaussian_then_median"},
                                 std::pair{"mean_then_gaussian", "gaussian_then_mean"}}) {
            const Image& a = cascades.at(pair.first);
            const Image& b = cascades.at(pair.second);
            order << pair.first << ',' << pair.second << ',' << meanAbsoluteDifference(a, b) << ','
                  << differentPixels(a, b) << ','
                  << meanAbsoluteDifference(toByteImage(a), toByteImage(b)) << ','
                  << differentPixels(toByteImage(a), toByteImage(b)) << '\n';
        }
        writeText(paths.output / "hw4_1_4" / "cascade_order.csv", order.str());
        std::cout << table.str() << order.str();
    }
} // namespace hw4
