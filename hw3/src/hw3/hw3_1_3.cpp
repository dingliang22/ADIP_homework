
#include "hw3/hw3_1_3.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace hw3 {

    namespace {

        using CDF = std::array<double, 256>;
        using LUT = std::array<Pixel, 256>;

        // ============================================================
        // 1. Calculate Normalized CDF
        // ============================================================

        CDF calculateCDF(const Histogram& histogram, size_t totalPixels) {
            if (totalPixels == 0) {
                throw std::invalid_argument("Cannot calculate CDF for an empty image.");
            }

            CDF cdf{};

            uint64_t cumulative = 0;

            for (int i = 0; i < 256; ++i) {
                cumulative += histogram[i];

                cdf[i] = static_cast<double>(cumulative) / static_cast<double>(totalPixels);
            }

            return cdf;
        }

        // ============================================================
        // 2. Build Histogram Matching LUT
        //
        // T(k) = argmin_j |CDF_source(k) - CDF_reference(j)|
        // ============================================================

        LUT buildMatchingLUT(const CDF& sourceCDF, const CDF& referenceCDF) {
            LUT lut{};

            for (int i = 0; i < 256; ++i) {
                double minDifference = std::numeric_limits<double>::max();

                int bestMatch = 0;

                // Find the reference gray level with the
                // closest cumulative probability.
                for (int j = 0; j < 256; ++j) {
                    const double difference = std::abs(sourceCDF[i] - referenceCDF[j]);

                    if (difference < minDifference) {
                        minDifference = difference;
                        bestMatch = j;
                    }
                }

                lut[i] = static_cast<Pixel>(bestMatch);
            }

            return lut;
        }

        // ============================================================
        // 3. Get maximum histogram count
        // ============================================================

        uint32_t getHistogramMax(const Histogram& histogram) {
            return *std::max_element(histogram.begin(), histogram.end());
        }

    } // anonymous namespace

    // ============================================================
    // 4. Histogram Matching
    // ============================================================

    GrayImage histogramMatching(const GrayImage& source, const GrayImage& reference) {
        if (!source.valid() || !reference.valid()) {
            throw std::invalid_argument("Invalid image for Histogram Matching.");
        }

        // Step 1: Calculate histograms using common library.
        const Histogram sourceHist = calculateHistogram(source);

        const Histogram referenceHist = calculateHistogram(reference);

        // Step 2: Calculate normalized CDFs.
        const CDF sourceCDF = calculateCDF(sourceHist, source.pixels.size());

        const CDF referenceCDF = calculateCDF(referenceHist, reference.pixels.size());

        // Step 3: Build Histogram Matching LUT.
        const LUT lut = buildMatchingLUT(sourceCDF, referenceCDF);

        // Step 4: Apply LUT to source image.
        // Keep the source image dimensions.
        GrayImage result = source;

        for (Pixel& pixel : result.pixels) {
            pixel = lut[pixel];
        }

        return result;
    }

    // ============================================================
    // 5. HW3 1-3 Entry Point
    // ============================================================

    void runHw3_1_3(const fs::path& inputFolder, const fs::path& outputRoot) {
        const fs::path outputFolder = outputRoot / "q3";

        fs::create_directories(outputFolder);

        std::cout << "\n";
        std::cout << "========================================\n";
        std::cout << "        HW3 1-3 Histogram Matching      \n";
        std::cout << "========================================\n";

        std::cout << "Input: " << inputFolder << '\n';

        std::cout << "Output: " << outputFolder << "\n\n";

        // --------------------------------------------------------
        // Step 1: Read source image (Planck 403x596)
        // --------------------------------------------------------

        const GrayImage planck = readRawGray(inputFolder / "Planck403x596.raw", 403, 596);

        // --------------------------------------------------------
        // Step 2: Read reference image (Liuchuan 800x500)
        // --------------------------------------------------------

        const GrayImage liuchuan = readRawGray(inputFolder / "Liuchuan800x500.raw", 800, 500);

        // --------------------------------------------------------
        // Step 3: Apply Histogram Matching
        // --------------------------------------------------------

        std::cout << "Matching Planck to Liuchuan...\n";

        const GrayImage matched = histogramMatching(planck, liuchuan);

        // --------------------------------------------------------
        // Step 4: Calculate histograms manually
        // --------------------------------------------------------

        const Histogram planckHist = calculateHistogram(planck);

        const Histogram liuchuanHist = calculateHistogram(liuchuan);

        const Histogram matchedHist = calculateHistogram(matched);

        // --------------------------------------------------------
        // Step 5: Save grayscale images
        // --------------------------------------------------------

        saveGrayPng(outputFolder / "Planck_original.png", planck);

        saveGrayPng(outputFolder / "Liuchuan_reference.png", liuchuan);

        saveGrayPng(outputFolder / "Planck_matched.png", matched);

        // --------------------------------------------------------
        // Step 6: Save histograms
        // --------------------------------------------------------

        saveHistogramPng(outputFolder / "Planck_hist_before.png", planckHist,
                         getHistogramMax(planckHist));

        saveHistogramPng(outputFolder / "Liuchuan_hist_reference.png", liuchuanHist,
                         getHistogramMax(liuchuanHist));

        saveHistogramPng(outputFolder / "Planck_hist_matched.png", matchedHist,
                         getHistogramMax(matchedHist));

        std::cout << "HW3 1-3 completed successfully!\n";
    }

} // namespace hw3
