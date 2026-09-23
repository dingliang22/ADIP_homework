#include <filesystem>
#include <opencv2/geometry.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
int main() {
  const std::string current_dir = std::filesystem::path(__FILE__).parent_path().string();
  const std::string filename = current_dir + "/fire.jpg";
  cv::Mat image = cv::imread(filename);
  cv::Scalar lowerBound(20, 220, 220);
  cv::Scalar upperBound(190, 255, 255);
  cv::Mat mask;
  cv::inRange(image, lowerBound, upperBound, mask);
  std::vector<std::vector<cv::Point>> contours;

  cv::findContours(
      mask,
      contours,
      cv::RETR_EXTERNAL,
      cv::CHAIN_APPROX_SIMPLE);

  // =========================
  // 畫 Bounding Box
  // =========================

  for (const auto &contour : contours) {
    // 過濾太小的雜訊
    double area = cv::contourArea(contour);

    if (area > 110) {
      cv::Rect box = cv::boundingRect(contour);

      // 畫在原圖上
      cv::rectangle(
          image,
          box,
          cv::Scalar(0, 0, 255), // 紅色
          5                      // 線寬
      );
    }
  }

  // =========================
  // 顯示圖片
  // =========================

  cv::namedWindow("window", cv::WINDOW_NORMAL);

  cv::imshow("window", image);

  cv::waitKey(0);
  cv::destroyAllWindows();

  return 0;
}