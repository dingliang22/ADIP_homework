#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stb/stb_image_write.h>
#include <string>
#include <vector>
using namespace std;
constexpr int WIDTH = 900;
constexpr int HEIGHT = 660;
constexpr size_t IMAGE_SIZE = static_cast<size_t>(WIDTH) * HEIGHT;

constexpr int BAYER_4X4[4][4] = {
    {0, 8, 2, 10},
    {12, 4, 14, 6},
    {3, 11, 1, 9},
    {15, 7, 13, 5},
};

bool loadRawImage(const string& filepath, vector<unsigned char>& image) {
    ifstream file(filepath, ios::binary);
    if (!file) {
        cerr << "Error: 無法開啟輸入檔案:\n" << filepath << '\n';
        return false;
    }
    image.resize(IMAGE_SIZE);
    file.read(reinterpret_cast<char*>(image.data()), static_cast<streamsize>(IMAGE_SIZE));
    if (file.gcount() != static_cast<streamsize>(IMAGE_SIZE)) {
        cerr << "Error: RAW 檔案大小不正確\n"
             << "預期大小: " << IMAGE_SIZE << " bytes\n"
             << "實際讀取: " << file.gcount() << " bytes\n";
        return false;
    }
    return true;
}

bool saveGrayPNG(const string& filepath, const vector<unsigned char>& image) {
    int result = stbi_write_png(filepath.c_str(), WIDTH, HEIGHT, 1, image.data(), WIDTH);
    if (result == 0) {
        cerr << "Error: PNG 輸出失敗:\n" << filepath << '\n';
        return false;
    }
    return true;
}

void quantize4Bit(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    dst.resize(src.size());
    constexpr int LEVELS = 16;
    for (size_t i = 0; i < src.size(); ++i) {
        int level = static_cast<int>(round(static_cast<double>(src[i]) * (LEVELS - 1) / 255.0));
        dst[i] =
            static_cast<unsigned char>(round(static_cast<double>(level) * 255.0 / (LEVELS - 1)));
    }
}

void quantize1Bit(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    dst.resize(src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        dst[i] = (src[i] >= 128) ? 255 : 0;
    }
}

void dither4Bit(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    dst.resize(src.size());
    constexpr int LEVELS = 16;
    constexpr float QUANT_STEP = 255.0f / (LEVELS - 1);
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            const size_t index = static_cast<size_t>(y) * WIDTH + x;
            float thresholdOffset =
                (static_cast<float>(BAYER_4X4[y % 4][x % 4]) + 0.5f) / 16.0f - 0.5f;
            float adjusted = static_cast<float>(src[index]) + thresholdOffset * QUANT_STEP;
            adjusted = clamp(adjusted, 0.0f, 255.0f);
            int level = static_cast<int>(round(adjusted * (LEVELS - 1) / 255.0f));
            level = clamp(level, 0, LEVELS - 1);
            dst[index] = static_cast<unsigned char>(
                round(static_cast<float>(level) * 255.0f / (LEVELS - 1)));
        }
    }
}

void dither1Bit(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    dst.resize(src.size());
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            const size_t index = static_cast<size_t>(y) * WIDTH + x;
            float threshold = (static_cast<float>(BAYER_4X4[y % 4][x % 4]) + 0.5f) / 16.0f * 255.0f;
            dst[index] = (static_cast<float>(src[index]) >= threshold) ? 255 : 0;
        }
    }
}

// ============================================================
// main
// ============================================================
int main() {
    const string current_dir = filesystem::path(__FILE__).parent_path().string();
    const string result_dir = current_dir + "/hw2_img_result";
    // 1. 原始 RAW
    const string input_path = current_dir + "/hw2_img/duck900x660.raw";
    // 2. 4-bit quantization
    const string output_4bit_quantized = result_dir + "/duck_4bit_quantized.png";
    // 3. 1-bit quantization
    const string output_1bit_quantized = result_dir + "/duck_1bit_quantized.png";
    // 4. 4-bit Bayer dithering
    const string output_4bit_dithered = result_dir + "/duck_4bit_dithered.png";
    // 5. 1-bit Bayer dithering
    const string output_1bit_dithered = result_dir + "/duck_1bit_dithered.png";

    vector<unsigned char> original;
    vector<unsigned char> quantized4Bit;
    vector<unsigned char> quantized1Bit;
    vector<unsigned char> dithered4Bit;
    vector<unsigned char> dithered1Bit;
    if (!loadRawImage(input_path, original)) {
        return -1;
    }

    quantize4Bit(original, quantized4Bit);
    quantize1Bit(original, quantized1Bit);

    dither4Bit(original, dithered4Bit);
    dither1Bit(original, dithered1Bit);
    // ========================================================
    // 4. Save PNG
    // ========================================================
    if (!saveGrayPNG(output_4bit_quantized, quantized4Bit)) {
        return -1;
    }
    if (!saveGrayPNG(output_1bit_quantized, quantized1Bit)) {
        return -1;
    }
    if (!saveGrayPNG(output_4bit_dithered, dithered4Bit)) {
        return -1;
    }
    if (!saveGrayPNG(output_1bit_dithered, dithered1Bit)) {
        return -1;
    }
    cout << "處理完成\n";
}