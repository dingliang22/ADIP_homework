#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
  const std::string current_dir = std::filesystem::absolute(__FILE__).parent_path().string();
  const std::string filename = current_dir + "/hw2_img/lena512_3.raw";

  const int width = 512;
  const int height = 512;
  cv::Mat img(height, width, CV_8UC1);

  // 以二進位模式讀取 .raw 檔案
  std::ifstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "錯誤：無法開啟檔案 " << filename << std::endl;
    return -1;
  }

  // 將資料寫入 cv::Mat 的記憶體空間
  file.read(reinterpret_cast<char *>(img.data), width * height);
  file.close();

  // 顯示影像
  cv::imshow("Lena RAW Image", img);
  cv::waitKey(0);

  return 0;
}