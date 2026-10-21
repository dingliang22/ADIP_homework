#include "hw3/common.hpp"

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>

namespace hw3 {

    // ============================================================
    // Save grayscale image as PNG
    // ============================================================

    void saveGrayPng(const fs::path& filePath, const GrayImage& image) {
        if (!image.valid()) {
            throw std::invalid_argument("Invalid image for PNG output.");
        }

        cv::Mat output(image.height, image.width, CV_8UC1);

        std::copy(image.pixels.begin(), image.pixels.end(), output.data);

        if (!cv::imwrite(filePath.string(), output)) {
            throw std::runtime_error("Failed to save PNG: " + filePath.string());
        }
    }

    // ============================================================
    // Draw Histogram using OpenCV
    // ============================================================

    void saveHistogramPng(const fs::path& filePath, const Histogram& histogram, uint32_t maxCount) {
        constexpr int canvasWidth = 880;
        constexpr int canvasHeight = 440;

        constexpr int left = 84;
        constexpr int top = 35;
        constexpr int plotWidth = 768;
        constexpr int plotHeight = 345;

        const int bottom = top + plotHeight;

        cv::Mat canvas(canvasHeight, canvasWidth, CV_8UC3, cv::Scalar(255, 255, 255));

        if (maxCount == 0) {
            maxCount = 1;
        }

        // Draw horizontal grid lines.
        for (int i = 0; i <= 4; ++i) {
            const int y = top + i * plotHeight / 4;

            cv::line(canvas, cv::Point(left, y), cv::Point(left + plotWidth, y),
                     cv::Scalar(225, 225, 225), 1);

            const uint32_t count =
                static_cast<uint32_t>(static_cast<uint64_t>(maxCount) * (4 - i) / 4);

            cv::putText(canvas, std::to_string(count), cv::Point(5, y + 5),
                        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
        }

        // Draw 256 histogram bins.
        for (int i = 0; i < 256; ++i) {
            if (histogram[i] == 0) {
                continue;
            }

            const int x = left + i * 3;

            const int barHeight = std::max(
                1, static_cast<int>(static_cast<double>(histogram[i]) / maxCount * plotHeight));

            cv::rectangle(canvas, cv::Point(x, bottom - barHeight), cv::Point(x + 2, bottom - 1),
                          cv::Scalar(60, 60, 60), cv::FILLED);
        }

        // Draw axes.
        cv::line(canvas, cv::Point(left, top), cv::Point(left, bottom), cv::Scalar(0, 0, 0), 2);

        cv::line(canvas, cv::Point(left, bottom), cv::Point(left + plotWidth, bottom),
                 cv::Scalar(0, 0, 0), 2);

        // X-axis labels.
        for (int gray : {0, 64, 128, 192, 255}) {
            const int x = left + gray * 3;

            cv::putText(canvas, std::to_string(gray), cv::Point(x - 12, bottom + 23),
                        cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(0, 0, 0), 1, cv::LINE_AA);
        }

        cv::putText(canvas, "Gray Level", cv::Point(400, 432), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(0, 0, 0), 1, cv::LINE_AA);

        cv::putText(canvas, "Pixel Count", cv::Point(5, 18), cv::FONT_HERSHEY_SIMPLEX, 0.45,
                    cv::Scalar(0, 0, 0), 1, cv::LINE_AA);

        if (!cv::imwrite(filePath.string(), canvas)) {
            throw std::runtime_error("Failed to save Histogram: " + filePath.string());
        }
    }

} // namespace hw3