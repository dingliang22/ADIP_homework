#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <stb/stb_image_write.h>

constexpr int ORIG_W = 512;
constexpr int ORIG_H = 512;
constexpr int TARG_W = 1024;
constexpr int TARG_H = 1024;

using namespace std;

int main() {
    // 定義路徑
    const string current_dir = filesystem::path(__FILE__).parent_path().string();

    const string output_filepath = current_dir + "/hw2_img_result/lena1024_combination.png";

    const string lena0_path = current_dir + "/hw2_img/lena512_0.raw";

    const string lena1_path = current_dir + "/hw2_img/lena512_1.raw";

    const string lena2_path = current_dir + "/hw2_img/lena512_2.raw";

    const string lena3_path = current_dir + "/hw2_img/lena512_3.raw";

    // 定義陣列
    vector<unsigned char> lena0(ORIG_H * ORIG_W);
    vector<unsigned char> lena1(ORIG_H * ORIG_W);
    vector<unsigned char> lena2(ORIG_H * ORIG_W);
    vector<unsigned char> lena3(ORIG_H * ORIG_W);

    vector<int> lena1024(TARG_W * TARG_H, 0);

    // 讀檔
    ifstream l0(lena0_path, ios::binary);
    ifstream l1(lena1_path, ios::binary);
    ifstream l2(lena2_path, ios::binary);
    ifstream l3(lena3_path, ios::binary);

    if (!l0 || !l1 || !l2 || !l3) {
        cerr << "讀檔出錯\n";
        return -1;
    }

    l0.read(reinterpret_cast<char*>(lena0.data()), static_cast<streamsize>(lena0.size()));

    l1.read(reinterpret_cast<char*>(lena1.data()), static_cast<streamsize>(lena1.size()));

    l2.read(reinterpret_cast<char*>(lena2.data()), static_cast<streamsize>(lena2.size()));

    l3.read(reinterpret_cast<char*>(lena3.data()), static_cast<streamsize>(lena3.size()));

    if (!l0 || !l1 || !l2 || !l3) {
        cerr << "讀取 RAW 資料失敗，檔案大小可能不正確\n";
        return -1;
    }

    l0.close();
    l1.close();
    l2.close();
    l3.close();

    vector<unsigned char>* lr_images[4] = {&lena0, &lena1, &lena2, &lena3};

    int dx[4] = {0, 1, 0, 1};
    int dy[4] = {0, 0, 1, 1};

    for (int Y = 0; Y < TARG_H; Y++) {
        for (int X = 0; X < TARG_W; X++) {
            int sum = 0;
            int count = 0;

            for (int k = 0; k < 4; k++) {
                int x = static_cast<int>(floor((X - dx[k]) / 2.0));

                int y = static_cast<int>(floor((Y - dy[k]) / 2.0));

                if (x >= 0 && x < ORIG_W && y >= 0 && y < ORIG_H) {
                    int lr_index = y * ORIG_W + x;

                    sum += (*lr_images[k])[lr_index];
                    count++;
                }
            }

            if (count > 0) {
                lena1024[Y * TARG_W + X] = sum / count;
            }
        }
    }

    // 轉成 8-bit grayscale buffer
    vector<unsigned char> out_buffer(TARG_W * TARG_H);

    for (int i = 0; i < TARG_W * TARG_H; i++) {
        out_buffer[i] = static_cast<unsigned char>(lena1024[i]);
    }

    // 確保輸出目錄存在
    filesystem::create_directories(filesystem::path(output_filepath).parent_path());

    // 寫成 PNG
    //
    // stbi_write_png(
    //     filename,
    //     width,
    //     height,
    //     components,
    //     data,
    //     stride_in_bytes
    // )
    //
    // 灰階影像 components = 1
    // 每一列有 TARG_W bytes
    int result =
        stbi_write_png(output_filepath.c_str(), TARG_W, TARG_H, 1, out_buffer.data(), TARG_W);

    if (result == 0) {
        cerr << "Error: PNG 寫入失敗\n";
        return -1;
    }

    cout << "影像已成功儲存至: " << output_filepath << '\n';

    return 0;
}