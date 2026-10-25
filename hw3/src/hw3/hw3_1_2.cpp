#include "hw3/hw3_1_2.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace hw3 {

    namespace {

        // ============================================================
        // 1. Read a pixel with replicated boundary handling
        // ============================================================

        Pixel getPixel(const GrayImage& image, int x, int y) {
            x = std::clamp(x, 0, image.width - 1);
            y = std::clamp(y, 0, image.height - 1);

            return image.pixels[static_cast<size_t>(y) * image.width + x];
        }

        // ============================================================
        // 2. Equalize one pixel using its local histogram
        // ============================================================

        Pixel equalizeLocalPixel(Pixel centerPixel, const Histogram& histogram, int totalPixels) {
            uint32_t cdf = 0;
            uint32_t cdfMin = 0;

            // Find the first nonzero histogram count.
            for (int i = 0; i < 256; ++i) {
                if (histogram[i] > 0) {
                    cdfMin = histogram[i];
                    break;
                }
            }

            // Calculate CDF at the center pixel intensity.
            for (int i = 0; i <= centerPixel; ++i) {
                cdf += histogram[i];
            }

            // All pixels in the local window have the same value.
            if (totalPixels == static_cast<int>(cdfMin)) {
                return centerPixel;
            }

            // Local histogram equalization formula.
            const double normalized =
                static_cast<double>(cdf - cdfMin) / static_cast<double>(totalPixels - cdfMin);

            const double mappedValue = normalized * 255.0;

            const int newGrayLevel = std::clamp(static_cast<int>(std::round(mappedValue)), 0, 255);

            return static_cast<Pixel>(newGrayLevel);
        }

        // ============================================================
        // 3. Get maximum histogram count
        // ============================================================

        uint32_t getHistogramMax(const Histogram& histogram) {
            return *std::max_element(histogram.begin(), histogram.end());
        }

        // ============================================================
        // 4. Process one image
        // ============================================================

        void processImage(const fs::path& inputFolder, const fs::path& outputFolder,
                          const std::string& name, int width, int height) {
            std::cout << "Processing: " << name << '\n';

            // Step 1: Read original RAW image.
            const GrayImage original = readRawGray(inputFolder / (name + ".raw"), width, height);

            // Step 2: Calculate original histogram.
            const Histogram originalHist = calculateHistogram(original);

            // Step 3: Save original image and histogram.
            saveGrayPng(outputFolder / (name + "_original.png"), original);

            saveHistogramPng(outputFolder / (name + "_hist_before.png"), originalHist,
                             getHistogramMax(originalHist));

            // Step 4: Process both window sizes.
            for (int windowSize : {5, 13}) {
                std::cout << "  Window: " << windowSize << "x" << windowSize << '\n';

                // Apply Local Histogram Equalization.
                const GrayImage enhanced = localHistogramEqualization(original, windowSize);

                // Calculate enhanced histogram.
                const Histogram enhancedHist = calculateHistogram(enhanced);

                const std::string suffix =
                    "_local_" + std::to_string(windowSize) + "x" + std::to_string(windowSize);

                // Save enhanced image.
                saveGrayPng(outputFolder / (name + suffix + ".png"), enhanced);

                // Save enhanced histogram.
                saveHistogramPng(outputFolder / (name + suffix + "_hist.png"), enhancedHist,
                                 getHistogramMax(enhancedHist));
            }

            std::cout << "Finished: " << name << "\n\n";
        }

    } // anonymous namespace

    // ============================================================
    // 5. Local Histogram Equalization
    //    Sliding Histogram Implementation
    // ============================================================

    GrayImage localHistogramEqualization(const GrayImage& image, int windowSize) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for Local Histogram Equalization.");
        }

        if (windowSize <= 0 || windowSize % 2 == 0) {
            throw std::invalid_argument("Window size must be a positive odd number.");
        }

        const int radius = windowSize / 2;
        const int totalPixels = windowSize * windowSize;

        GrayImage result = image;

        // Process each image row.
        for (int y = 0; y < image.height; ++y) {
            Histogram localHist{};

            // ----------------------------------------------------
            // Initialize histogram for the first pixel (x = 0).
            // ----------------------------------------------------

            for (int dy = -radius; dy <= radius; ++dy) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    const Pixel pixel = getPixel(image, dx, y + dy);

                    localHist[pixel]++;
                }
            }

            // ----------------------------------------------------
            // Move the window horizontally.
            // ----------------------------------------------------

            for (int x = 0; x < image.width; ++x) {
                if (x > 0) {
                    // Remove the leftmost column
                    // of the previous window.
                    const int removeX = x - radius - 1;

                    // Add the new rightmost column.
                    const int addX = x + radius;

                    for (int dy = -radius; dy <= radius; ++dy) {
                        const int currentY = y + dy;

                        const Pixel removePixel = getPixel(image, removeX, currentY);

                        const Pixel addPixel = getPixel(image, addX, currentY);

                        localHist[removePixel]--;
                        localHist[addPixel]++;
                    }
                }

                // ------------------------------------------------
                // Equalize only the center pixel.
                // ------------------------------------------------

                const size_t index = static_cast<size_t>(y) * image.width + x;

                result.pixels[index] =
                    equalizeLocalPixel(image.pixels[index], localHist, totalPixels);
            }
        }

        return result;
    }

    // ============================================================
    // 6. HW3 1-2 Entry Point
    // ============================================================

    void runHw3_1_2(const fs::path& inputFolder, const fs::path& outputRoot) {
        const fs::path outputFolder = outputRoot / "q2";

        fs::create_directories(outputFolder);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "   HW3 1-2 Local Histogram Equalization \n";
        std::cout << "========================================\n";

        std::cout << "Input: " << inputFolder << '\n';

        std::cout << "Output: " << outputFolder << "\n\n";

        // Liuchuan 800x500
        processImage(inputFolder, outputFolder, "Liuchuan800x500", 800, 500);

        // Planck 403x596
        processImage(inputFolder, outputFolder, "Planck403x596", 403, 596);

        std::cout << "HW3 1-2 completed successfully!\n";
    }

} // namespace hw3
