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

// 原圖大小
constexpr int ORIG_W = 512;
constexpr int ORIG_H = 512;

// 目標大小
constexpr int TARGET_W = 1024;
constexpr int TARGET_H = 1024;

// 最近鄰插值法
void nearestNeighborResize(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    float scaleX = static_cast<float>(ORIG_W) / TARGET_W;
    float scaleY = static_cast<float>(ORIG_H) / TARGET_H;

    for (int y = 0; y < TARGET_H; ++y) {
        for (int x = 0; x < TARGET_W; ++x) {
            // 映射回原圖座標
            int srcX = static_cast<int>(round(x * scaleX));

            int srcY = static_cast<int>(round(y * scaleY));

            // 邊界保護
            srcX = min(srcX, ORIG_W - 1);
            srcY = min(srcY, ORIG_H - 1);

            dst[y * TARGET_W + x] = src[srcY * ORIG_W + srcX];
        }
    }
}

// 雙線性插值法
void bilinearResize(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    float scaleX = static_cast<float>(ORIG_W) / TARGET_W;
    float scaleY = static_cast<float>(ORIG_H) / TARGET_H;

    for (int y = 0; y < TARGET_H; ++y) {
        for (int x = 0; x < TARGET_W; ++x) {
            // 映射回原圖的浮點座標
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            // 左上座標
            int x1 = static_cast<int>(floor(srcX));
            int y1 = static_cast<int>(floor(srcY));

            // 右下座標
            int x2 = min(x1 + 1, ORIG_W - 1);
            int y2 = min(y1 + 1, ORIG_H - 1);

            // 邊界保護
            x1 = min(x1, ORIG_W - 1);
            y1 = min(y1, ORIG_H - 1);

            // 小數部分，也就是插值權重
            float dx = srcX - x1;
            float dy = srcY - y1;

            // 四個鄰近像素
            float p1 = src[y1 * ORIG_W + x1]; // 左上
            float p2 = src[y1 * ORIG_W + x2]; // 右上
            float p3 = src[y2 * ORIG_W + x1]; // 左下
            float p4 = src[y2 * ORIG_W + x2]; // 右下

            // Bilinear interpolation
            float interp = (1.0f - dx) * (1.0f - dy) * p1 + dx * (1.0f - dy) * p2 +
                           (1.0f - dx) * dy * p3 + dx * dy * p4;

            dst[y * TARGET_W + x] = static_cast<unsigned char>(round(interp));
        }
    }
}

int main() {
    const string current_dir = filesystem::path(__FILE__).parent_path().string();
    const string img_without_LP = current_dir + "/hw2_img/lena512_0.raw";
    const string nn_output_path = current_dir + "/hw2_img_result/lena1024_nearest.png";
    const string bilinear_output_path = current_dir + "/hw2_img_result/lena1024_bilinear.png";
    vector<unsigned char> unfiltered_img(ORIG_W * ORIG_H);
    ifstream file1(img_without_LP, ios::binary);

    if (!file1) {
        cerr << "無法開啟輸入檔案\n";
        return -1;
    }

    // 讀取第一張
    file1.read(reinterpret_cast<char*>(unfiltered_img.data()),
               static_cast<streamsize>(unfiltered_img.size()));

    file1.close();

    // 建立輸出資料夾
    filesystem::create_directories(current_dir + "/hw2_img_result");

    // 建立 1024 x 1024 buffer
    vector<unsigned char> nnImage(TARGET_W * TARGET_H);

    vector<unsigned char> bilinearImage(TARGET_W * TARGET_H);

    // 使用沒有 Low Pass Filter 的影像
    nearestNeighborResize(unfiltered_img, nnImage);

    bilinearResize(unfiltered_img, bilinearImage);

    int nn_result =
        stbi_write_png(nn_output_path.c_str(), TARGET_W, TARGET_H, 1, nnImage.data(), TARGET_W);

    if (nn_result == 0) {
        cerr << "Nearest Neighbor PNG 輸出失敗\n";

        return -1;
    }

    // --------------------------------
    // 輸出 Bilinear PNG
    // --------------------------------

    int bilinear_result = stbi_write_png(bilinear_output_path.c_str(), TARGET_W, TARGET_H, 1,
                                         bilinearImage.data(), TARGET_W);

    if (bilinear_result == 0) {
        cerr << "Bilinear PNG 輸出失敗\n";

        return -1;
    }

    cout << "Nearest Neighbor 圖片已輸出至:\n" << nn_output_path << "\n\n";

    cout << "Bilinear 圖片已輸出至:\n" << bilinear_output_path << '\n';

    return 0;
}