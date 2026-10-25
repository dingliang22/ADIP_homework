#include "hw3/hw3_4_2.hpp"

#include "hw3/watermark_bits.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace hw3 {

    namespace {

        using Block = std::array<Pixel, 64>;

        // ============================================================
        // Validate QIM parameters
        // ============================================================

        void checkQIMImage(const GrayImage& image, int delta) {
            if (!image.valid() || image.width % 8 != 0 || image.height % 8 != 0) {
                throw std::invalid_argument("QIM requires dimensions divisible by 8.");
            }

            if (delta <= 0 || delta > 128 || 256 % delta != 0) {
                throw std::invalid_argument("Delta must be a divisor of 256 in [1,128].");
            }
        }

        // ============================================================
        // Read an 8x8 block
        // ============================================================

        Block readBlock(const GrayImage& image, int x0, int y0) {
            Block block{};

            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    block[y * 8 + x] =
                        image.pixels[static_cast<size_t>(y0 + y) * image.width + x0 + x];
                }
            }

            return block;
        }

        // ============================================================
        // Write an 8x8 block
        // ============================================================

        void writeBlock(GrayImage& image, int x0, int y0, const Block& block) {
            for (int y = 0; y < 8; ++y) {
                for (int x = 0; x < 8; ++x) {
                    image.pixels[static_cast<size_t>(y0 + y) * image.width + x0 + x] =
                        block[y * 8 + x];
                }
            }
        }

        // ============================================================
        // Calculate the median of 64 grayscale values
        // ============================================================

        double medianOf(const Block& block) {
            Block sorted = block;

            std::sort(sorted.begin(), sorted.end());

            return (static_cast<double>(sorted[31]) + static_cast<double>(sorted[32])) / 2.0;
        }

        // ============================================================
        // Shift all 64 pixels by the same amount
        // ============================================================

        Block shiftedBlock(const Block& block, int amount) {
            Block result{};

            for (int i = 0; i < 64; ++i) {
                result[i] =
                    static_cast<Pixel>(std::clamp(static_cast<int>(block[i]) + amount, 0, 255));
            }

            return result;
        }

        // ============================================================
        // Try to move block median into target quantization bin
        // ============================================================

        bool embedTargetIndex(const Block& block, int targetQ, int delta, Block& output) {
            const double m = medianOf(block);

            const int targetMedian = targetQ * delta;

            const int idealShift = static_cast<int>(std::ceil(targetMedian - m));

            // Try the ideal shift first.
            // If clipping changes the median, search nearby shifts.
            for (int radius = 0; radius <= 255; ++radius) {
                for (int direction : {0, 1}) {
                    if (radius == 0 && direction == 1) {
                        continue;
                    }

                    const int shift = idealShift + (direction == 0 ? -radius : radius);

                    if (shift < -255 || shift > 255) {
                        continue;
                    }

                    const Block candidate = shiftedBlock(block, shift);

                    const int actualQ = static_cast<int>(medianOf(candidate) / delta);

                    if (actualQ == targetQ) {
                        output = candidate;
                        return true;
                    }
                }
            }

            return false;
        }

    } // anonymous namespace

    // ============================================================
    // Embed repeated ASCII student ID using Block Median QIM
    // ============================================================

    GrayImage embedBlockMedianQIM(const GrayImage& image, const std::string& id, int delta) {
        validateStudentId(id);
        checkQIMImage(image, delta);

        GrayImage result = image;

        size_t blockIndex = 0;

        const int maxQ = 255 / delta;

        // Traverse blocks in raster scan order.
        for (int y = 0; y < image.height; y += 8) {
            for (int x = 0; x < image.width; x += 8) {
                const Block original = readBlock(image, x, y);

                const double m = medianOf(original);

                const int q = static_cast<int>(m / delta);

                const Pixel watermarkBit = repeatedIdBit(id, blockIndex);

                ++blockIndex;

                // Already matches requested bit.
                if ((q & 1) == watermarkBit) {
                    continue;
                }

                // Check q-1 and q+1.
                std::vector<int> choices;

                if (q - 1 >= 0) {
                    choices.push_back(q - 1);
                }

                if (q + 1 <= maxQ) {
                    choices.push_back(q + 1);
                }

                // Choose the target closest to original median.
                std::sort(choices.begin(), choices.end(), [&](int a, int b) {
                    const double da = std::abs(a * delta - m);

                    const double db = std::abs(b * delta - m);

                    return da == db ? a < b : da < db;
                });

                bool embedded = false;

                for (int targetQ : choices) {
                    Block modified{};

                    if (embedTargetIndex(original, targetQ, delta, modified)) {
                        writeBlock(result, x, y, modified);

                        embedded = true;
                        break;
                    }
                }

                if (!embedded) {
                    throw std::runtime_error("Cannot embed QIM bit in block " +
                                             std::to_string(blockIndex - 1));
                }
            }
        }

        return result;
    }

    // ============================================================
    // Decode one watermark bit from each 8x8 block
    // ============================================================

    std::vector<Pixel> decodeQIMBits(const GrayImage& image, int delta) {
        checkQIMImage(image, delta);

        std::vector<Pixel> bits;

        bits.reserve(static_cast<size_t>(image.width / 8) * (image.height / 8));

        for (int y = 0; y < image.height; y += 8) {
            for (int x = 0; x < image.width; x += 8) {
                const double m = medianOf(readBlock(image, x, y));

                const int q = static_cast<int>(m / delta);

                bits.push_back(static_cast<Pixel>(q & 1));
            }
        }

        return bits;
    }

    // ============================================================
    // Main task: HW3 4-2
    // ============================================================

    void runHw3_4_2(const fs::path& inputFolder, const fs::path& root, const std::string& id) {
        constexpr int delta = 8;

        const fs::path out = root / "watermark_qim";

        fs::create_directories(out);

        const GrayImage original = readRawGray(inputFolder / "baboon512.raw", 512, 512);

        // Embed QIM watermark.
        const GrayImage encoded = embedBlockMedianQIM(original, id, delta);

        // Save required RAW file.
        const fs::path raw = out / ("baboon512_QIM_" + id + ".raw");

        writeRawGray(raw, encoded);

        // Read back and verify.
        const GrayImage reloaded = readRawGray(raw, 512, 512);

        const auto report = analyzeDecodedBits(decodeQIMBits(reloaded, delta), id);

        printDecodeReport("QIM before distortion (Delta=8)", report);

        if (report.bitErrors != 0) {
            throw std::runtime_error("QIM verification failed.");
        }

        std::cout << "  PSNR: " << calculatePSNR(original, reloaded) << " dB\n";

        // Calculate original and watermarked histograms.
        const auto before = calculateHistogram(original);
        const auto after = calculateHistogram(reloaded);

        const uint32_t scale = std::max(*std::max_element(before.begin(), before.end()),
                                        *std::max_element(after.begin(), after.end()));

        // Save PNG figures.
        saveGrayPng(out / "baboon512_original.png", original);

        saveGrayPng(out / ("baboon512_QIM_" + id + ".png"), reloaded);

        saveHistogramPng(out / "baboon512_hist_before.png", before, scale);

        saveHistogramPng(out / "baboon512_hist_after.png", after, scale);

        std::cout << "Saved: " << raw << '\n';
    }

} // namespace hw3