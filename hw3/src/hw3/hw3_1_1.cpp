
#include "hw3/hw3_1_1.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace hw3 {

    // ============================================================
    // 1. Global Histogram Equalization
    // ============================================================

    GrayImage histogramEqualization(const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for histogram equalization.");
        }

        // Calculate histogram using common library.
        const Histogram histogram = calculateHistogram(image);

        const size_t totalPixels = image.pixels.size();

        // Step 1: Calculate CDF.
        std::array<size_t, 256> cdf{};

        size_t cumulative = 0;

        for (int i = 0; i < 256; ++i) {
            cumulative += histogram[i];
            cdf[i] = cumulative;
        }

        // Step 2: Find the minimum non-zero CDF.
        size_t cdfMin = 0;

        for (int i = 0; i < 256; ++i) {
            if (histogram[i] != 0) {
                cdfMin = cdf[i];
                break;
            }
        }

        // If every pixel has the same gray level,
        // return the original image.
        if (totalPixels == cdfMin) {
            return image;
        }

        // Step 3: Build equalization LUT.
        std::array<Pixel, 256> lut{};

        for (int i = 0; i < 256; ++i) {
            // Avoid unsigned underflow for gray levels
            // below the first non-zero histogram bin.
            if (cdf[i] <= cdfMin) {
                lut[i] = 0;
                continue;
            }

            const double normalized =
                static_cast<double>(cdf[i] - cdfMin) / static_cast<double>(totalPixels - cdfMin);

            const double mappedValue = normalized * 255.0;

            const int newGrayLevel = std::clamp(static_cast<int>(std::round(mappedValue)), 0, 255);

            lut[i] = static_cast<Pixel>(newGrayLevel);
        }

        // Step 4: Apply LUT to every pixel.
        GrayImage result = image;

        for (Pixel& pixel : result.pixels) {
            pixel = lut[pixel];
        }

        return result;
    }

    // ============================================================
    // 2. Process One Image
    // ============================================================

    namespace {

        void processImage(const fs::path& inputFolder, const fs::path& outputFolder,
                          const std::string& name, int width, int height) {
            std::cout << "Processing: " << name << '\n';

            // Step 1: Read original RAW image.
            const GrayImage original = readRawGray(inputFolder / (name + ".raw"), width, height);

            // Step 2: Calculate original histogram.
            const Histogram histogramBefore = calculateHistogram(original);

            // Step 3: Apply Histogram Equalization.
            const GrayImage equalized = histogramEqualization(original);

            // Step 4: Calculate equalized histogram.
            const Histogram histogramAfter = calculateHistogram(equalized);

            // Step 5: Save original and equalized images.
            saveGrayPng(outputFolder / (name + "_original.png"), original);

            saveGrayPng(outputFolder / (name + "_equalized.png"), equalized);

            // Step 6: Determine histogram Y-axis scales.
            const uint32_t maxBefore =
                *std::max_element(histogramBefore.begin(), histogramBefore.end());

            const uint32_t maxAfter =
                *std::max_element(histogramAfter.begin(), histogramAfter.end());

            // Step 7: Save histograms.
            saveHistogramPng(outputFolder / (name + "_hist_before.png"), histogramBefore,
                             maxBefore);

            saveHistogramPng(outputFolder / (name + "_hist_after.png"), histogramAfter, maxAfter);

            std::cout << "Finished: " << name << "\n\n";
        }

    } // anonymous namespace

    // ============================================================
    // 3. HW3 1-1 Entry Point
    // ============================================================

    void runHw3_1_1(const fs::path& inputFolder, const fs::path& outputRoot) {
        const fs::path outputFolder = outputRoot / "q1";

        fs::create_directories(outputFolder);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "     HW3 1-1 Histogram Equalization     \n";
        std::cout << "========================================\n";

        std::cout << "Input: " << inputFolder << '\n';
        std::cout << "Output: " << outputFolder << "\n\n";

        // Liuchuan 800x500
        processImage(inputFolder, outputFolder, "Liuchuan800x500", 800, 500);

        // Planck 403x596
        processImage(inputFolder, outputFolder, "Planck403x596", 403, 596);

        std::cout << "HW3 1-1 completed successfully!\n";
    }

} // namespace hw3
