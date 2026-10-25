
#include "hw3/hw3_3_1.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace hw3 {

    namespace {

        // ============================================================
        // 1. Process One Image
        // ============================================================

        void processImage(const fs::path& inputFolder, const fs::path& outputFolder,
                          const std::string& name, int width, int height) {
            constexpr double targetPSNR = 35.0;

            std::cout << "Processing: " << name << '\n';

            // --------------------------------------------------------
            // Step 1: Read original RAW image
            // --------------------------------------------------------

            const GrayImage original = readRawGray(inputFolder / (name + ".raw"), width, height);

            // --------------------------------------------------------
            // Step 2: Find the minimum k with PSNR >= 35 dB
            // --------------------------------------------------------

            int selectedK = 0;
            double selectedPSNR = 0.0;

            GrayImage reconstructed;

            for (int k = 1; k <= 8; ++k) {
                // Keep only the top k MSB planes.
                GrayImage candidate = keepTopKPlanes(original, k);

                // PSNR calculation is provided by common/metrics.cpp.
                const double psnr = calculatePSNR(original, candidate);

                std::cout << "  k = " << k << ", PSNR = " << std::fixed << std::setprecision(4)
                          << psnr << " dB\n";

                if (psnr >= targetPSNR) {
                    selectedK = k;
                    selectedPSNR = psnr;

                    reconstructed = std::move(candidate);
                    break;
                }
            }

            if (selectedK == 0) {
                throw std::runtime_error("Cannot find a valid k.");
            }

            // --------------------------------------------------------
            // Step 3: Print selected bit-planes
            // --------------------------------------------------------

            std::cout << "\nSelected k: " << selectedK << '\n';

            std::cout << "Selected bit-planes: ";

            for (int bit = 7; bit >= 8 - selectedK; --bit) {
                std::cout << bit << ' ';
            }

            std::cout << "\nSelected PSNR: " << selectedPSNR << " dB\n\n";

            // --------------------------------------------------------
            // Step 4: Calculate histograms manually
            // --------------------------------------------------------

            const Histogram histBefore = calculateHistogram(original);

            const Histogram histAfter = calculateHistogram(reconstructed);

            // Use a common Y-axis scale.
            const uint32_t maxCount =
                std::max(*std::max_element(histBefore.begin(), histBefore.end()),
                         *std::max_element(histAfter.begin(), histAfter.end()));

            // --------------------------------------------------------
            // Step 5: Save original and reconstructed images
            // --------------------------------------------------------

            saveGrayPng(outputFolder / (name + "_original.png"), original);

            saveGrayPng(outputFolder / (name + "_msb_reconstructed.png"), reconstructed);

            // --------------------------------------------------------
            // Step 6: Save histograms
            // --------------------------------------------------------

            saveHistogramPng(outputFolder / (name + "_hist_before.png"), histBefore, maxCount);

            saveHistogramPng(outputFolder / (name + "_hist_after.png"), histAfter, maxCount);

            std::cout << "Finished: " << name << "\n\n";
        }

    } // anonymous namespace

    // ============================================================
    // 2. Keep Top k MSB Bit-planes
    //
    // Example:
    // k = 5
    // mask = 11111000
    //
    // Reconstructed pixel = Original pixel & mask
    // ============================================================

    GrayImage keepTopKPlanes(const GrayImage& image, int k) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for MSB selection.");
        }

        if (k < 1 || k > 8) {
            throw std::invalid_argument("k must be between 1 and 8.");
        }

        // Build the bit mask manually.
        const Pixel mask = static_cast<Pixel>(0xFFu << (8 - k));

        GrayImage result = image;

        // Preserve only the selected MSB planes.
        for (Pixel& pixel : result.pixels) {
            pixel = static_cast<Pixel>(pixel & mask);
        }

        return result;
    }

    // ============================================================
    // 3. HW3 3-1 Entry Point
    // ============================================================

    void runHw3_3_1(const fs::path& inputFolder, const fs::path& outputRoot) {
        const fs::path outputFolder = outputRoot / "quantization_msb";

        fs::create_directories(outputFolder);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "       HW3 3-1 Top-k MSB Selection      \n";
        std::cout << "========================================\n";

        std::cout << "Input: " << inputFolder << '\n';

        std::cout << "Output: " << outputFolder << "\n\n";

        // Lena 256x256
        processImage(inputFolder, outputFolder, "lena256", 256, 256);

        // Baboon 512x512
        processImage(inputFolder, outputFolder, "baboon512", 512, 512);

        std::cout << "HW3 3-1 completed successfully!\n";
    }

} // namespace hw3
