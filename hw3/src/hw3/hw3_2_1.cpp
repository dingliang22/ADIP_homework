
#include "hw3/hw3_2_1.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace hw3 {

    namespace {

        // ============================================================
        // 1. Calculate mean gray level
        // ============================================================

        double calculateMean(const GrayImage& image) {
            if (!image.valid()) {
                throw std::invalid_argument("Invalid image for mean calculation.");
            }

            double sum = 0.0;

            for (Pixel pixel : image.pixels) {
                sum += pixel;
            }

            return sum / image.pixels.size();
        }

        // ============================================================
        // 2. Process one image
        // ============================================================

        void processImage(const fs::path& inputFolder, const fs::path& outputFolder,
                          const std::string& name, int width, int height) {
            std::cout << "Processing: " << name << '\n';

            // --------------------------------------------------------
            // Step 1: Read original RAW image
            // --------------------------------------------------------

            const GrayImage original = readRawGray(inputFolder / (name + ".raw"), width, height);

            // --------------------------------------------------------
            // Step 2: Apply Negative Transformation
            // --------------------------------------------------------

            const GrayImage negative = negativeTransformation(original);

            // --------------------------------------------------------
            // Step 3: Calculate histograms manually
            // --------------------------------------------------------

            const Histogram histBefore = calculateHistogram(original);

            const Histogram histAfter = calculateHistogram(negative);

            // Use the same Y-axis scale for before and after.
            const uint32_t maxCount =
                std::max(*std::max_element(histBefore.begin(), histBefore.end()),
                         *std::max_element(histAfter.begin(), histAfter.end()));

            // --------------------------------------------------------
            // Step 4: Save original and negative images
            // --------------------------------------------------------

            saveGrayPng(outputFolder / (name + "_original.png"), original);

            saveGrayPng(outputFolder / (name + "_negative.png"), negative);

            // --------------------------------------------------------
            // Step 5: Save histograms
            // --------------------------------------------------------

            saveHistogramPng(outputFolder / (name + "_hist_before.png"), histBefore, maxCount);

            saveHistogramPng(outputFolder / (name + "_hist_after.png"), histAfter, maxCount);

            // --------------------------------------------------------
            // Step 6: Print mean gray levels
            // --------------------------------------------------------

            std::cout << "  Mean before: " << calculateMean(original) << '\n';

            std::cout << "  Mean after : " << calculateMean(negative) << '\n';

            std::cout << "Finished: " << name << "\n\n";
        }

    } // anonymous namespace

    // ============================================================
    // 3. Negative Gray-Level Transformation
    //
    // s = 255 - r
    // ============================================================

    GrayImage negativeTransformation(const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for Negative Transformation.");
        }

        GrayImage result = image;

        for (Pixel& pixel : result.pixels) {
            pixel = static_cast<Pixel>(255 - pixel);
        }

        return result;
    }

    // ============================================================
    // 4. HW3 2-1 Entry Point
    // ============================================================

    void runHw3_2_1(const fs::path& inputFolder, const fs::path& outputRoot) {
        const fs::path outputFolder = outputRoot / "gray_negative";

        fs::create_directories(outputFolder);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "  HW3 2-1 Negative Gray Transformation  \n";
        std::cout << "========================================\n";

        std::cout << "Input: " << inputFolder << '\n';

        std::cout << "Output: " << outputFolder << "\n\n";

        // Dark image: 600x400
        processImage(inputFolder, outputFolder, "dark600x400", 600, 400);

        // Light image: 512x341
        processImage(inputFolder, outputFolder, "light512x341", 512, 341);

        std::cout << "HW3 2-1 completed successfully!\n";
    }

} // namespace hw3
