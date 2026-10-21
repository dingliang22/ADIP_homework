#include "hw3/common.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace hw3 {

    double calculatePSNR(const GrayImage& original, const GrayImage& processed) {
        if (!original.valid() || !processed.valid() || original.width != processed.width ||
            original.height != processed.height) {
            throw std::invalid_argument("Invalid or mismatched image dimensions.");
        }

        double squaredError = 0.0;

        for (size_t i = 0; i < original.pixels.size(); ++i) {
            const double diff =
                static_cast<double>(original.pixels[i]) - static_cast<double>(processed.pixels[i]);

            squaredError += diff * diff;
        }

        const double mse = squaredError / original.pixels.size();

        if (mse == 0.0) {
            return std::numeric_limits<double>::infinity();
        }

        return 10.0 * std::log10(255.0 * 255.0 / mse);
    }

} // namespace hw3