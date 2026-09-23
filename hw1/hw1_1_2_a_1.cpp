#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
int main() {
  // 定義圖片長、寬與圖片位置
  const int width = 256;
  const int height = 256;
  std::string current_dir = std::filesystem::path(__FILE__).parent_path().string();
  std::string filename = current_dir + "/lena256.raw";
  // 讀檔
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "[-] 無法開啟檔案: " << filename << std::endl;
    std::cerr << "    錯誤原因: " << std::strerror(errno) << std::endl;
    return 1;
  }
  // 用一個1為陣列存放讀進來的數據
  std::vector<unsigned char> image(width * height);
  file.read(reinterpret_cast<char *>(image.data()), width * height);
  file.close();

  // 迴圈找像素點
  bool found = false;
  for (int row = 0; row < height; ++row) {
    for (int col = 0; col < width; ++col) {
      int index = row * width + col;

      if (image[index] == 232) {
        std::cout << "找到第一個強度值為 232 的像素" << std::endl;
        std::cout << "座標 (row, column) = (" << row << ", " << col << ")" << std::endl;
        found = true;
        break;
      }
    }
    if (found) break;
  }

  if (!found) {
    std::cout << "在影像中找不到數值為 232 的像素。" << std::endl;
  }

  return 0;
}