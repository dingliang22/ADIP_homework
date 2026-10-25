
#include "hw3/hw3_3_2.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace hw3 {

    namespace {

        // ============================================================
        // 1. Save All Eight Bit-planes in a 2x4 PNG
        // ============================================================

        void saveBitPlaneOverview(const fs::path& filePath, const GrayImage& image) {
            if (!image.valid()) {
                throw std::invalid_argument("Invalid image for Bit-plane overview.");
            }

            constexpr int columns = 4;
            constexpr int rows = 2;
            constexpr int margin = 8;
            constexpr int labelHeight = 30;
            constexpr int bottomMargin = 10;

            const int width = image.width;
            const int height = image.height;

            const int cellWidth = width + margin * 2;

            const int cellHeight = height + labelHeight + bottomMargin;

            // Create the overview canvas.
            cv::Mat overview(rows * cellHeight, columns * cellWidth, CV_8UC1, cv::Scalar(255));

            // Arrange Bit 7 to Bit 0.
            for (int bit = 7; bit >= 0; --bit) {
                const int position = 7 - bit;

                const int row = position / columns;
                const int col = position % columns;

                const int x = col * cellWidth + margin;

                const int y = row * cellHeight + labelHeight;

                // Extract the current bit-plane.
                GrayImage plane = extractBitPlane(image, bit);

                // Create an OpenCV view for visualization.
                // No pixel transformation is performed here.
                cv::Mat planeMat(height, width, CV_8UC1, plane.pixels.data());

                // Copy the plane into its overview cell.
                planeMat.copyTo(overview(cv::Rect(x, y, width, height)));

                // Draw the bit-plane label.
                cv::putText(overview, "Bit " + std::to_string(bit), cv::Point(x, y - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(0), 1, cv::LINE_AA);
            }

            // This is a composed visualization,
            // so it is written directly with OpenCV.
            if (!cv::imwrite(filePath.string(), overview)) {
                throw std::runtime_error("Failed to save Bit-plane overview: " + filePath.string());
            }
        }

        // ============================================================
        // 2. Process One Image
        // ============================================================

        void processImage(const fs::path& inputFolder, const fs::path& outputFolder,
                          const std::string& name, int width, int height,
                          const std::vector<int>& manualBits) {
            constexpr double coherenceThreshold = 0.10;

            std::cout << "Processing: " << name << '\n';

            // --------------------------------------------------------
            // Step 1: Read original RAW image
            // --------------------------------------------------------

            const GrayImage original = readRawGray(inputFolder / (name + ".raw"), width, height);

            // Save the original image.
            saveGrayPng(outputFolder / (name + "_original.png"), original);

            // --------------------------------------------------------
            // Step 2: Save all eight bit-plane images
            // --------------------------------------------------------

            for (int bit = 7; bit >= 0; --bit) {
                const GrayImage plane = extractBitPlane(original, bit);

                saveGrayPng(outputFolder / (name + "_bit" + std::to_string(bit) + ".png"), plane);
            }

            // --------------------------------------------------------
            // Step 3: Save the 2x4 bit-plane overview
            // --------------------------------------------------------

            saveBitPlaneOverview(outputFolder / (name + "_all_8_bitplanes.png"), original);

            // --------------------------------------------------------
            // Step 4: Select meaningful bit-planes
            // --------------------------------------------------------

            std::vector<int> selectedBits;

            if (manualBits.empty()) {
                std::cout << "Automatic selection:\n";

                selectedBits = selectMeaningfulPlanes(original, coherenceThreshold);

            } else {
                std::cout << "Manual selection enabled.\n";

                selectedBits = manualBits;
            }

            // Validate all selected bit indices.
            for (int bit : selectedBits) {
                if (bit < 0 || bit > 7) {
                    throw std::invalid_argument("Selected bit index must be between 0 and 7.");
                }
            }

            // Remove duplicates and sort in descending order.
            std::sort(selectedBits.begin(), selectedBits.end(), [](int a, int b) { return a > b; });

            selectedBits.erase(std::unique(selectedBits.begin(), selectedBits.end()),
                               selectedBits.end());

            // --------------------------------------------------------
            // Step 5: Reconstruct from selected bit-planes
            // --------------------------------------------------------

            const GrayImage reconstructed = reconstructFromPlanes(original, selectedBits);

            // Calculate PSNR using common/metrics.cpp.
            const double psnr = calculatePSNR(original, reconstructed);

            // --------------------------------------------------------
            // Step 6: Print selection statistics
            // --------------------------------------------------------

            std::cout << "\nSelected bit-planes: ";

            for (int bit : selectedBits) {
                std::cout << bit << ' ';
            }

            std::cout << "\nNumber of selected planes: " << selectedBits.size();

            std::cout << "\nReconstructed PSNR: " << std::fixed << std::setprecision(4) << psnr
                      << " dB\n\n";

            // --------------------------------------------------------
            // Step 7: Calculate histograms
            // --------------------------------------------------------

            const Histogram histBefore = calculateHistogram(original);

            const Histogram histAfter = calculateHistogram(reconstructed);

            // Use the same Y-axis scale.
            const uint32_t maxCount =
                std::max(*std::max_element(histBefore.begin(), histBefore.end()),
                         *std::max_element(histAfter.begin(), histAfter.end()));

            // --------------------------------------------------------
            // Step 8: Save reconstructed image
            // --------------------------------------------------------

            saveGrayPng(outputFolder / (name + "_structure_reconstructed.png"), reconstructed);

            // --------------------------------------------------------
            // Step 9: Save histograms
            // --------------------------------------------------------

            saveHistogramPng(outputFolder / (name + "_hist_before.png"), histBefore, maxCount);

            saveHistogramPng(outputFolder / (name + "_hist_after.png"), histAfter, maxCount);

            std::cout << "Finished: " << name << "\n\n";
        }

    } // anonymous namespace

    // ============================================================
    // 3. Extract One Bit-plane
    //
    // Example:
    // Bit 7 = 1 -> 255
    // Bit 7 = 0 -> 0
    // ============================================================

    GrayImage extractBitPlane(const GrayImage& image, int bit) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for Bit-plane extraction.");
        }

        if (bit < 0 || bit > 7) {
            throw std::invalid_argument("Bit index must be between 0 and 7.");
        }

        const Pixel mask = static_cast<Pixel>(1u << bit);

        GrayImage plane = image;

        for (Pixel& pixel : plane.pixels) {
            pixel = (pixel & mask) ? 255 : 0;
        }

        return plane;
    }

    // ============================================================
    // 4. Calculate Spatial Coherence
    //
    // Compare neighboring bits horizontally and vertically.
    //
    // Expected switch rate for independent bits:
    //     2 * p * (1 - p)
    //
    // Coherence:
    //     1 - observedSwitchRate / expectedSwitchRate
    //
    // Larger positive values suggest spatial structure.
    // ============================================================

    PlaneStats calculatePlaneStats(const GrayImage& image, int bit) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for Bit-plane statistics.");
        }

        if (bit < 0 || bit > 7) {
            throw std::invalid_argument("Bit index must be between 0 and 7.");
        }

        const Pixel mask = static_cast<Pixel>(1u << bit);

        uint64_t ones = 0;
        uint64_t switches = 0;
        uint64_t comparisons = 0;

        const int width = image.width;
        const int height = image.height;

        // --------------------------------------------------------
        // Count 1-bits and neighboring switches
        // --------------------------------------------------------

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const size_t index = static_cast<size_t>(y) * width + x;

                const bool current = (image.pixels[index] & mask) != 0;

                if (current) {
                    ++ones;
                }

                // Compare with the right neighbor.
                if (x + 1 < width) {
                    const bool right = (image.pixels[index + 1] & mask) != 0;

                    if (current != right) {
                        ++switches;
                    }

                    ++comparisons;
                }

                // Compare with the bottom neighbor.
                if (y + 1 < height) {
                    const bool bottom = (image.pixels[index + width] & mask) != 0;

                    if (current != bottom) {
                        ++switches;
                    }

                    ++comparisons;
                }
            }
        }

        // --------------------------------------------------------
        // Calculate statistics
        // --------------------------------------------------------

        const double p = static_cast<double>(ones) / static_cast<double>(image.pixels.size());

        const double switchRate =
            comparisons > 0 ? static_cast<double>(switches) / static_cast<double>(comparisons)
                            : 0.0;

        const double expectedSwitchRate = 2.0 * p * (1.0 - p);

        // Constant bit-planes do not contain useful structure.
        if (expectedSwitchRate < 1e-12) {
            return {p, switchRate, 0.0};
        }

        const double coherence = 1.0 - switchRate / expectedSwitchRate;

        return {p, switchRate, coherence};
    }

    // ============================================================
    // 5. Automatically Select Meaningful Bit-planes
    //
    // This is a heuristic, not an absolute quality rule.
    // ============================================================

    std::vector<int> selectMeaningfulPlanes(const GrayImage& image, double threshold) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for Bit-plane selection.");
        }

        std::vector<int> selected;

        for (int bit = 7; bit >= 0; --bit) {
            const PlaneStats stats = calculatePlaneStats(image, bit);

            std::cout << "  Bit " << bit << " | One fraction: " << std::fixed
                      << std::setprecision(4) << stats.oneFraction
                      << " | Switch rate: " << stats.switchRate
                      << " | Coherence: " << stats.coherence;

            if (stats.coherence >= threshold) {
                selected.push_back(bit);

                std::cout << "  [SELECTED]";
            }

            std::cout << '\n';
        }

        // Fallback if no plane passes the threshold.
        if (selected.empty()) {
            selected.push_back(7);

            std::cout << "  No plane passed the threshold.\n"
                      << "  Fallback: selecting Bit 7.\n";
        }

        return selected;
    }

    // ============================================================
    // 6. Reconstruct Image From Selected Bit-planes
    // ============================================================

    GrayImage reconstructFromPlanes(const GrayImage& image, const std::vector<int>& selectedBits) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for Bit-plane reconstruction.");
        }

        Pixel mask = 0;

        // Combine selected bit-plane masks.
        for (int bit : selectedBits) {
            if (bit < 0 || bit > 7) {
                throw std::invalid_argument("Invalid selected bit-plane.");
            }

            mask = static_cast<Pixel>(mask | static_cast<Pixel>(1u << bit));
        }

        GrayImage reconstructed = image;

        // Apply the combined mask.
        for (Pixel& pixel : reconstructed.pixels) {
            pixel = static_cast<Pixel>(pixel & mask);
        }

        return reconstructed;
    }

    // ============================================================
    // 7. HW3 3-2 Entry Point
    // ============================================================

    void runHw3_3_2(const fs::path& inputFolder, const fs::path& outputRoot) {
        const fs::path outputFolder = outputRoot / "quantization_structure";

        fs::create_directories(outputFolder);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "    HW3 3-2 Bit-plane Structure Test    \n";
        std::cout << "========================================\n";

        std::cout << "Input: " << inputFolder << '\n';

        std::cout << "Output: " << outputFolder << "\n\n";

        // --------------------------------------------------------
        // Selection settings
        // --------------------------------------------------------

        const std::vector<int> lenaManualBits = {7, 6, 5, 4};
        const std::vector<int> baboonManualBits = {7, 6, 5};

        // --------------------------------------------------------
        // Process Lena
        // --------------------------------------------------------

        processImage(inputFolder, outputFolder, "lena256", 256, 256, lenaManualBits);

        // --------------------------------------------------------
        // Process Baboon
        // --------------------------------------------------------

        processImage(inputFolder, outputFolder, "baboon512", 512, 512, baboonManualBits);

        std::cout << "HW3 3-2 completed successfully!\n";
    }

} // namespace hw3
