#include "hw3/hw3_4_1.hpp"

#include "hw3/watermark_bits.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace hw3 {

    // Embed repeated student ID into every pixel's LSB.
    GrayImage embedLSB(const GrayImage& image, const std::string& id) {
        validateStudentId(id);

        if (!image.valid()) {
            throw std::invalid_argument("Invalid image.");
        }

        GrayImage output = image;

        for (size_t i = 0; i < output.pixels.size(); ++i) {
            output.pixels[i] =
                static_cast<Pixel>((output.pixels[i] & 0xFEu) | repeatedIdBit(id, i));
        }

        return output;
    }

    // Extract one bit from each pixel's LSB.
    std::vector<Pixel> decodeLSBBits(const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image.");
        }

        std::vector<Pixel> bits(image.pixels.size());

        for (size_t i = 0; i < image.pixels.size(); ++i) {
            bits[i] = image.pixels[i] & 1u;
        }

        return bits;
    }

    // Entry point for HW3 4-1.
    void runHw3_4_1(const fs::path& inputFolder, const fs::path& root, const std::string& id) {
        const fs::path out = root / "watermark_lsb";

        fs::create_directories(out);

        const GrayImage original = readRawGray(inputFolder / "baboon512.raw", 512, 512);

        // Embed student ID.
        const GrayImage encoded = embedLSB(original, id);

        // Save required RAW image.
        const fs::path raw = out / ("baboon512_" + id + ".raw");

        writeRawGray(raw, encoded);

        // Read RAW back to verify file correctness.
        const GrayImage reloaded = readRawGray(raw, 512, 512);

        // Decode the watermark.
        const auto report = analyzeDecodedBits(decodeLSBBits(reloaded), id);

        printDecodeReport("LSB before distortion", report);

        if (report.bitErrors != 0) {
            throw std::runtime_error("LSB verification failed.");
        }

        std::cout << "  PSNR: " << calculatePSNR(original, reloaded) << " dB\n";

        // Calculate Histogram manually.
        const auto before = calculateHistogram(original);
        const auto after = calculateHistogram(reloaded);

        const uint32_t scale = std::max(*std::max_element(before.begin(), before.end()),
                                        *std::max_element(after.begin(), after.end()));

        // Save PNG images and histograms.
        saveGrayPng(out / "baboon512_original.png", original);

        saveGrayPng(out / ("baboon512_" + id + ".png"), reloaded);

        saveHistogramPng(out / "baboon512_hist_before.png", before, scale);

        saveHistogramPng(out / "baboon512_hist_after.png", after, scale);

        std::cout << "Saved: " << raw << '\n';
    }

} // namespace hw3