#include "hw3/hw3_4_3.hpp"

#include "hw3/hw3_4_1.hpp"
#include "hw3/hw3_4_2.hpp"
#include "hw3/watermark_bits.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace hw3 {

    namespace {

        // ============================================================
        // Save encoded/degraded images and histograms
        // ============================================================

        void saveComparison(const fs::path& out, const std::string& prefix,
                            const GrayImage& encoded, const GrayImage& degraded) {
            // Save RAW files.
            writeRawGray(out / (prefix + "_encoded.raw"), encoded);

            writeRawGray(out / (prefix + "_6bit.raw"), degraded);

            // Save PNG images.
            saveGrayPng(out / (prefix + "_encoded.png"), encoded);

            saveGrayPng(out / (prefix + "_6bit.png"), degraded);

            // Calculate histograms.
            const auto before = calculateHistogram(encoded);
            const auto after = calculateHistogram(degraded);

            const uint32_t scale = std::max(*std::max_element(before.begin(), before.end()),
                                            *std::max_element(after.begin(), after.end()));

            // Save histograms.
            saveHistogramPng(out / (prefix + "_hist_encoded.png"), before, scale);

            saveHistogramPng(out / (prefix + "_hist_6bit.png"), after, scale);
        }

    } // anonymous namespace

    // ============================================================
    // 6-bit Quantization
    //
    // Keep the six MSBs.
    // Example: 10101101 -> 10101100
    // ============================================================

    GrayImage quantizeTo6Bit(const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid input image.");
        }

        GrayImage result = image;

        for (Pixel& value : result.pixels) {
            value = static_cast<Pixel>(value & 0xFCu);
        }

        return result;
    }

    // ============================================================
    // Main task: HW3 4-3
    // ============================================================

    void runHw3_4_3(const fs::path& inputFolder, const fs::path& root, const std::string& id) {
        validateStudentId(id);

        const fs::path out = root / "watermark_robustness";

        fs::create_directories(out);

        const GrayImage original = readRawGray(inputFolder / "baboon512.raw", 512, 512);

        // ========================================================
        // Part A: LSB Watermark
        // ========================================================

        const GrayImage lsb = embedLSB(original, id);

        // Apply 6-bit quantization.
        const GrayImage lsb6 = quantizeTo6Bit(lsb);

        saveComparison(out, "baboon512_LSB", lsb, lsb6);

        std::cout << "\n========== LSB ==========\n";

        // Decode before distortion.
        printDecodeReport("Before 6-bit quantization", analyzeDecodedBits(decodeLSBBits(lsb), id));

        // Decode after distortion.
        printDecodeReport("After 6-bit quantization", analyzeDecodedBits(decodeLSBBits(lsb6), id));

        std::cout << "  Embedded PSNR: " << calculatePSNR(original, lsb) << " dB\n";

        std::cout << "  Degraded PSNR: " << calculatePSNR(original, lsb6) << " dB\n";

        // ========================================================
        // Part B: QIM with different Delta values
        // ========================================================

        for (int delta : {4, 8, 16}) {
            // Embed watermark.
            const GrayImage qim = embedBlockMedianQIM(original, id, delta);

            // Apply 6-bit quantization.
            const GrayImage qim6 = quantizeTo6Bit(qim);

            const std::string prefix = "baboon512_QIM_delta" + std::to_string(delta);

            // Save RAW, PNG and histograms.
            saveComparison(out, prefix, qim, qim6);

            // Also save the required Delta=8 QIM RAW name.
            if (delta == 8) {
                writeRawGray(out / ("baboon512_QIM_" + id + ".raw"), qim);
            }

            std::cout << "\n========== QIM Delta=" << delta << " ==========\n";

            // Decode before distortion.
            printDecodeReport("Before 6-bit quantization",
                              analyzeDecodedBits(decodeQIMBits(qim, delta), id));

            // Decode after distortion.
            printDecodeReport("After 6-bit quantization",
                              analyzeDecodedBits(decodeQIMBits(qim6, delta), id));

            std::cout << "  Embedded PSNR: " << calculatePSNR(original, qim) << " dB\n";

            std::cout << "  Degraded PSNR: " << calculatePSNR(original, qim6) << " dB\n";
        }

        std::cout << "\nResults saved in: " << out << '\n';
    }

} // namespace hw3