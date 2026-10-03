#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// 原圖大小
constexpr int ORIG_W = 512;
constexpr int ORIG_H = 512;

// 目標縮小尺寸
constexpr int TARGET_W = 358;
constexpr int TARGET_H = 358;

// 最近鄰插值法 (Nearest Neighbor)
void nearestNeighborShrink(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    float scaleX = static_cast<float>(ORIG_W) / TARGET_W;

    float scaleY = static_cast<float>(ORIG_H) / TARGET_H;

    for (int y = 0; y < TARGET_H; ++y) {
        for (int x = 0; x < TARGET_W; ++x) {

            // 計算對應回原圖的座標
            int srcX = static_cast<int>(round(x * scaleX));

            int srcY = static_cast<int>(round(y * scaleY));

            // 邊界保護
            srcX = min(srcX, ORIG_W - 1);
            srcY = min(srcY, ORIG_H - 1);

            // 寫入目標影像
            dst[y * TARGET_W + x] = src[srcY * ORIG_W + srcX];
        }
    }
}

// 雙線性插值法 (Bilinear Interpolation)
void bilinearShrink(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    float scaleX = static_cast<float>(ORIG_W) / TARGET_W;

    float scaleY = static_cast<float>(ORIG_H) / TARGET_H;

    for (int y = 0; y < TARGET_H; ++y) {
        for (int x = 0; x < TARGET_W; ++x) {

            // 映射回原圖的浮點座標
            float srcX = x * scaleX;
            float srcY = y * scaleY;

            // 左上
            int x1 = static_cast<int>(floor(srcX));

            int y1 = static_cast<int>(floor(srcY));

            // 右下
            int x2 = min(x1 + 1, ORIG_W - 1);

            int y2 = min(y1 + 1, ORIG_H - 1);

            // 邊界保護
            x1 = min(x1, ORIG_W - 1);
            y1 = min(y1, ORIG_H - 1);

            // 計算距離權重
            float dx = srcX - x1;
            float dy = srcY - y1;

            // 四個鄰近像素
            float p1 = src[y1 * ORIG_W + x1];

            float p2 = src[y1 * ORIG_W + x2];

            float p3 = src[y2 * ORIG_W + x1];

            float p4 = src[y2 * ORIG_W + x2];

            // 雙線性插值
            float interp = (1.0f - dx) * (1.0f - dy) * p1 + dx * (1.0f - dy) * p2 +
                           (1.0f - dx) * dy * p3 + dx * dy * p4;

            // 四捨五入後寫入
            dst[y * TARGET_W + x] = static_cast<unsigned char>(round(interp));
        }
    }
}

// 輸出 8-bit 灰階 PNG
bool saveGrayPNG(const string& path, const vector<unsigned char>& image, int width, int height) {
    int result = stbi_write_png(path.c_str(), width, height, 1, image.data(), width);

    return result != 0;
}

int main() {
    const string current_dir = filesystem::absolute(__FILE__).parent_path().string();

    // 輸入檔案
    const string img_without_LP = current_dir + "/hw2_img/lena512_0.raw";
    const string img_with_LP = current_dir + "/hw2_img/lena512_0_with_LP.raw";

    // 輸出資料夾
    const string output_dir = current_dir + "/hw2_img_result";
    filesystem::create_directories(output_dir);

    // 輸出檔名
    const string nn_without_LP_path = output_dir + "/lena358_nearest_without_LP.png";
    const string bilinear_without_LP_path = output_dir + "/lena358_bilinear_without_LP.png";
    const string nn_with_LP_path = output_dir + "/lena358_nearest_with_LP.png";
    const string bilinear_with_LP_path = output_dir + "/lena358_bilinear_with_LP.png";
    // 建立輸入影像 buffer
    vector<unsigned char> unfiltered_img(ORIG_W * ORIG_H);
    vector<unsigned char> filtered_img(ORIG_W * ORIG_H);

    // 讀檔
    ifstream file1(img_without_LP, ios::binary);

    ifstream file2(img_with_LP, ios::binary);

    if (!file1 || !file2) {
        cerr << "無法開啟輸入檔案\n";
        return -1;
    }

    // 讀取未做 LP 的圖片
    file1.read(reinterpret_cast<char*>(unfiltered_img.data()),
               static_cast<streamsize>(unfiltered_img.size()));

    // 讀取有做 LP 的圖片
    file2.read(reinterpret_cast<char*>(filtered_img.data()),
               static_cast<streamsize>(filtered_img.size()));

    file1.close();
    file2.close();

    vector<unsigned char> nnImage(TARGET_W * TARGET_H);
    vector<unsigned char> bilinearImage(TARGET_W * TARGET_H);
    nearestNeighborShrink(unfiltered_img, nnImage);
    bilinearShrink(unfiltered_img, bilinearImage);
    if (!saveGrayPNG(nn_without_LP_path, nnImage, TARGET_W, TARGET_H)) {
        cerr << "輸出失敗: " << nn_without_LP_path << '\n';

        return -1;
    }

    if (!saveGrayPNG(bilinear_without_LP_path, bilinearImage, TARGET_W, TARGET_H)) {
        cerr << "輸出失敗: " << bilinear_without_LP_path << '\n';

        return -1;
    }

    nearestNeighborShrink(filtered_img, nnImage);
    bilinearShrink(filtered_img, bilinearImage);
    if (!saveGrayPNG(nn_with_LP_path, nnImage, TARGET_W, TARGET_H)) {
        cerr << "輸出失敗: " << nn_with_LP_path << '\n';

        return -1;
    }
    if (!saveGrayPNG(bilinear_with_LP_path, bilinearImage, TARGET_W, TARGET_H)) {
        cerr << "輸出失敗: " << bilinear_with_LP_path << '\n';

        return -1;
    }
    cout << "輸出完成";
    return 0;
}