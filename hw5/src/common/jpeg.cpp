#include "hw5/common.hpp"
#include <opencv2/core.hpp>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace hw5 {
QTable baseQuantTable() {
    return {16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
            14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
            18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
            49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};
}

// S = 5000/QF (QF < 50), 200 - 2QF (QF >= 50); Q_QF = clamp(floor((S*Q + 50) / 100), 1, 255).
QTable qualityTable(const QTable& base, int qf) {
    if (qf < 1 || qf > 100) throw std::invalid_argument("QF must be in [1,100]");
    const double s = qf < 50 ? 5000.0 / qf : 200.0 - 2.0 * qf;
    QTable table(64);
    for (int i = 0; i < 64; ++i)
        table[i] = std::clamp(static_cast<int>(std::floor((s * base[i] + 50.0) / 100.0)), 1, 255);
    return table;
}

// 8x8 block pipeline: level shift -128 -> cv::dct -> round(F/Q) -> F_q*Q -> cv::idct -> +128.
// Entropy coding is intentionally omitted (not required).
JpegResult jpegSimulate(const Image& image, const QTable& table) {
    if (image.width % 8 || image.height % 8) throw std::invalid_argument("Image must be 8x8 aligned");
    JpegResult result{Image(image.width, image.height), 0};
    cv::Mat block(8, 8, CV_64F), coefficients, restored;
    for (int by = 0; by < image.height; by += 8)
        for (int bx = 0; bx < image.width; bx += 8) {
            for (int y = 0; y < 8; ++y)
                for (int x = 0; x < 8; ++x) block.at<double>(y, x) = image.at(bx + x, by + y) - 128.0;
            cv::dct(block, coefficients);
            for (int y = 0; y < 8; ++y)
                for (int x = 0; x < 8; ++x) {
                    const int q = table[y * 8 + x];
                    const double level = std::round(coefficients.at<double>(y, x) / q);
                    if (level != 0) ++result.nonZeroCoefficients;
                    coefficients.at<double>(y, x) = level * q;
                }
            cv::idct(coefficients, restored);
            for (int y = 0; y < 8; ++y)
                for (int x = 0; x < 8; ++x)
                    result.reconstructed.at(bx + x, by + y) = restored.at<double>(y, x) + 128.0;
        }
    result.reconstructed = toByteImage(result.reconstructed);   // decoded JPEG is 8-bit
    return result;
}

std::string formatTable(const QTable& table) {
    std::ostringstream out;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) out << std::setw(4) << table[y * 8 + x];
        out << '\n';
    }
    return out.str();
}
} // namespace hw5
