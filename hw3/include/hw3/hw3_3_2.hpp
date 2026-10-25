#pragma once

#include "hw3/common.hpp"

#include <vector>

namespace hw3 {

    // Statistics of one bit-plane.
    struct PlaneStats {
        double oneFraction = 0.0;
        double switchRate = 0.0;
        double coherence = 0.0;
    };

    // Extract one bit-plane for visualization.
    // Pixel values are either 0 or 255.
    GrayImage extractBitPlane(const GrayImage& image, int bit);

    // Calculate spatial coherence for one bit-plane.
    PlaneStats calculatePlaneStats(const GrayImage& image, int bit);

    // Automatically select meaningful bit-planes.
    std::vector<int> selectMeaningfulPlanes(const GrayImage& image, double threshold);

    // Reconstruct an image from selected bit-planes.
    GrayImage reconstructFromPlanes(const GrayImage& image, const std::vector<int>& selectedBits);

    // Entry point for HW3 3-2.
    void runHw3_3_2(const fs::path& inputFolder, const fs::path& outputRoot);

} // namespace hw3