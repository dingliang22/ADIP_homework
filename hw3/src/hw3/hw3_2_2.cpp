
#include "hw3/hw3_2_2.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace hw3 {

namespace {

// ============================================================
// 1. Calculate Mean Gray Level
// ============================================================

double calculateMean(const GrayImage& image) {
    if (!image.valid()) {
        throw std::invalid_argument(
            "Invalid image for mean calculation."
        );
    }

    double sum = 0.0;

    for (Pixel pixel : image.pixels) {
        sum += pixel;
    }

    return sum / image.pixels.size();
}

// ============================================================
// 2. Process One Image
// ============================================================

void processImage(
    const fs::path& inputFolder,
    const fs::path& outputFolder,
    const std::string& name,
    int width,
    int height
) {
    std::cout << "Processing: " << name << '\n';

    // --------------------------------------------------------
    // Step 1: Read original RAW image
    // --------------------------------------------------------

    const GrayImage original = readRawGray(
        inputFolder / (name + ".raw"),
        width,
        height
    );

    // --------------------------------------------------------
    // Step 2: Apply Power-law Transformation
    // --------------------------------------------------------

    // Gamma < 1: Brighten the image
    const GrayImage gamma05 =
        powerLawTransformation(original, 0.5);

    // Gamma > 1: Darken the image
    const GrayImage gamma20 =
        powerLawTransformation(original, 2.0);

    // --------------------------------------------------------
    // Step 3: Calculate Histograms Manually
    // --------------------------------------------------------

    const Histogram histOriginal =
        calculateHistogram(original);

    const Histogram histGamma05 =
        calculateHistogram(gamma05);

    const Histogram histGamma20 =
        calculateHistogram(gamma20);

    // --------------------------------------------------------
    // Step 4: Use a Common Y-axis Scale
    // --------------------------------------------------------

    uint32_t maxCount = 0;

    for (const Histogram* hist :
         {&histOriginal, &histGamma05, &histGamma20}) {

        const uint32_t currentMax =
            *std::max_element(
                hist->begin(),
                hist->end()
            );

        maxCount = std::max(
            maxCount,
            currentMax
        );
    }

    // --------------------------------------------------------
    // Step 5: Save Original Image and Histogram
    // --------------------------------------------------------

    saveGrayPng(
        outputFolder / (name + "_original.png"),
        original
    );

    saveHistogramPng(
        outputFolder / (name + "_hist_before.png"),
        histOriginal,
        maxCount
    );

    // --------------------------------------------------------
    // Step 6: Save Gamma = 0.5 Results
    // --------------------------------------------------------

    saveGrayPng(
        outputFolder / (name + "_gamma_0p5.png"),
        gamma05
    );

    saveHistogramPng(
        outputFolder / (name + "_gamma_0p5_hist.png"),
        histGamma05,
        maxCount
    );

    // --------------------------------------------------------
    // Step 7: Save Gamma = 2.0 Results
    // --------------------------------------------------------

    saveGrayPng(
        outputFolder / (name + "_gamma_2p0.png"),
        gamma20
    );

    saveHistogramPng(
        outputFolder / (name + "_gamma_2p0_hist.png"),
        histGamma20,
        maxCount
    );

    // --------------------------------------------------------
    // Step 8: Print Mean Gray Levels
    // --------------------------------------------------------

    std::cout << "  Original mean  : "
              << calculateMean(original) << '\n';

    std::cout << "  Gamma 0.5 mean : "
              << calculateMean(gamma05) << '\n';

    std::cout << "  Gamma 2.0 mean : "
              << calculateMean(gamma20) << '\n';

    std::cout << "Finished: " << name << "\n\n";
}

} // anonymous namespace

// ============================================================
// 3. Power-law (Gamma) Transformation
//
// s = 255 * (r / 255)^gamma
// ============================================================

GrayImage powerLawTransformation(
    const GrayImage& image,
    double gamma
) {
    if (!image.valid()) {
        throw std::invalid_argument(
            "Invalid image for Power-law Transformation."
        );
    }

    if (!std::isfinite(gamma) || gamma <= 0.0) {
        throw std::invalid_argument(
            "Gamma must be a finite positive number."
        );
    }

    // --------------------------------------------------------
    // Step 1: Build Lookup Table
    // --------------------------------------------------------

    std::array<Pixel, 256> lut{};

    for (int i = 0; i < 256; ++i) {
        const double normalized =
            static_cast<double>(i) / 255.0;

        const double transformed =
            255.0 * std::pow(normalized, gamma);

        const int grayLevel = std::clamp(
            static_cast<int>(std::round(transformed)),
            0,
            255
        );

        lut[i] = static_cast<Pixel>(grayLevel);
    }

    // --------------------------------------------------------
    // Step 2: Apply LUT to Each Pixel
    // --------------------------------------------------------

    GrayImage result = image;

    for (Pixel& pixel : result.pixels) {
        pixel = lut[pixel];
    }

    return result;
}

// ============================================================
// 4. HW3 2-2 Entry Point
// ============================================================

void runHw3_2_2(
    const fs::path& inputFolder,
    const fs::path& outputRoot
) {
    const fs::path outputFolder =
        outputRoot / "gray_gamma";

    fs::create_directories(outputFolder);

    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "    HW3 2-2 Power-law Transformation    \n";
    std::cout << "========================================\n";

    std::cout << "Input: "
              << inputFolder << '\n';

    std::cout << "Output: "
              << outputFolder << "\n\n";

    // Dark image: 600x400
    processImage(
        inputFolder,
        outputFolder,
        "dark600x400",
        600,
        400
    );

    // Light image: 512x341
    processImage(
        inputFolder,
        outputFolder,
        "light512x341",
        512,
        341
    );

    std::cout
        << "HW3 2-2 completed successfully!\n";
}

} // namespace hw3
