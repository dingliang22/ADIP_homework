#include "hw3/common.hpp"

#include <stdexcept>

namespace hw3 {

    Histogram calculateHistogram(const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for histogram.");
        }

        Histogram histogram{};

        for (Pixel pixel : image.pixels) {
            histogram[pixel]++;
        }

        return histogram;
    }

} // namespace hw3