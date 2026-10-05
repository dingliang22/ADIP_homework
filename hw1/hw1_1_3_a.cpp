#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
const int width = 256;
const int height = 256;

int main() {
    std::string current_dir = std::filesystem::path(__FILE__).parent_path().string();
    std::string filename = current_dir + "/lena256.raw";
    std::string out_name = current_dir + "/lena256_brightness.raw";
    std::vector<unsigned char> image(width * height);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "讀檔錯誤" << std::strerror(errno) << std::endl;
        return 1;
    }
    file.read(reinterpret_cast<char*>(image.data()), width * height);
    file.close();
    for (auto& pixel : image) {
        int random = (rand() % 121) - 60;
        int tmp = pixel + random;
        pixel = tmp < 0 ? 0 : ((tmp > 255) ? 255 : tmp);
    }
    std::ofstream outFile(out_name, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(image.data()), width * height);
        outFile.close();
        std::cout << "[+] 檔案寫入完成: " << out_name << std::endl;
    } else {
        std::cerr << "[-] 無法開啟檔案進行寫入" << std::endl;
        std::cerr << "    錯誤原因: " << std::strerror(errno) << std::endl;
        return 1;
    }
    return 0;
}