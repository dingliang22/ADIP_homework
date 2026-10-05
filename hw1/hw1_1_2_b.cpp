#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const int width = 256;
const int height = 256;
const int BLOCK_SIZE = 128; // 子區塊大小為 128x128

// 1. 定義形狀類型 (輸出邊界條件)
enum ShapeType {
    FULL_SQUARE, // 完整正方形 (128x128)
    TRI_UPPER,   // 主對角線上半部 (c >= r)
    TRI_LOWER    // 主對角線下半部 (c < r)
};

// 2. 定義單一翻轉與旋轉模式 (Opcode)
enum TransformMode {
    NONE = 0, // 不變
    ROT_90,   // 順時針 90 度
    ROT_180,  // 順時針 180 度
    ROT_270,  // 順時針 270 度
    FLIP_H,   // 水平翻轉
    FLIP_V    // 垂直翻轉
};

// 3. 定義區域 (Region) 結構
struct Region {
    int baseR;       // 區塊起始 R 座標 (0 或 128)
    int baseC;       // 區塊起始 C 座標 (0 或 128)
    ShapeType shape; // 形狀範圍
};

void applySingleBackwardTransform(int& r, int& c, TransformMode mode) {
    int origR = r, origC = c;
    switch (mode) {
        case ROT_90:
            origR = 127 - c;
            origC = r;
            break;
        case ROT_180:
            origR = 127 - r;
            origC = 127 - c;
            break;
        case ROT_270:
            origR = c;
            origC = 127 - r;
            break;
        case FLIP_H:
            origR = r;
            origC = 127 - c;
            break;
        case FLIP_V:
            origR = 127 - r;
            origC = c;
            break;
        default:
            break; // NONE
    }
    r = origR;
    c = origC;
}

void processBlock(const unsigned char srcImg[height][width], Region srcRegion,
                  unsigned char dstImg[height][width], Region dstRegion,
                  const std::vector<TransformMode>& modes) {
    for (int r = 0; r < BLOCK_SIZE; ++r) {
        for (int c = 0; c < BLOCK_SIZE; ++c) {
            // Step 1: 檢查是否落在目標形狀內部 (輸出的邊界條件)
            bool inDstShape = false;
            if (dstRegion.shape == FULL_SQUARE) {
                inDstShape = true;
            } else if (dstRegion.shape == TRI_UPPER) {
                inDstShape = (c >= r);
            } else if (dstRegion.shape == TRI_LOWER) {
                inDstShape = (c < r);
            }

            if (!inDstShape) {
                continue; // 不在目標形狀內直接跳過
            }

            // Step 2: 多重反向映射 (從最後一個操作逆向推回原始座標)
            int origR = r, origC = c;
            for (auto it = modes.rbegin(); it != modes.rend(); ++it) {
                applySingleBackwardTransform(origR, origC, *it);
            }

            // Step 3: 計算絕對座標並填入輸出陣列
            int dstAbsR = dstRegion.baseR + r;
            int dstAbsC = dstRegion.baseC + c;
            int srcAbsR = srcRegion.baseR + origR;
            int srcAbsC = srcRegion.baseC + origC;

            dstImg[dstAbsR][dstAbsC] = srcImg[srcAbsR][srcAbsC];
        }
    }
}

int main() {
    // 定義圖片位置
    std::string current_dir = std::filesystem::path(__FILE__).parent_path().string();
    std::string filename = current_dir + "/lena256.raw";
    std::string outname = current_dir + "/lena256_out.raw";

    // 1. 讀檔
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[-] 無法開啟檔案: " << filename << std::endl;
        std::cerr << "    錯誤原因: " << std::strerror(errno) << std::endl;
        return 1;
    }

    unsigned char image[height][width];
    unsigned char out_image[height][width] = {0}; // 初始化輸出矩陣

    file.read(reinterpret_cast<char*>(image), width * height);
    file.close();

    Region SRC_Q0 = {0, 0, FULL_SQUARE};     // 左上
    Region SRC_Q1 = {0, 128, FULL_SQUARE};   // 右上
    Region SRC_Q2 = {128, 0, FULL_SQUARE};   // 左下
    Region SRC_Q3 = {128, 128, FULL_SQUARE}; // 右下

    Region DST_Q0 = {0, 0, FULL_SQUARE};     // 左上正方形
    Region DST_Q2 = {128, 0, FULL_SQUARE};   // 左下正方形
    Region DST_Q3 = {128, 128, FULL_SQUARE}; // 右下正方形

    Region DST_Q1_UP = {0, 128, TRI_UPPER}; // 右上象限 (主對角線上半部)
    Region DST_Q1_LO = {0, 128, TRI_LOWER}; // 右上象限 (主對角線下半部)

    processBlock(image, SRC_Q2, out_image, DST_Q0, {NONE});
    processBlock(image, SRC_Q0, out_image, DST_Q2, {NONE});
    processBlock(image, SRC_Q3, out_image, DST_Q3, {ROT_270});
    processBlock(image, SRC_Q1, out_image, DST_Q1_UP, {ROT_270, FLIP_V});
    processBlock(image, SRC_Q1, out_image, DST_Q1_LO, {ROT_180});

    std::ofstream outFile(outname, std::ios::binary);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(out_image), width * height);
        outFile.close();
        std::cout << "[+] 檔案寫入完成: " << outname << std::endl;
    } else {
        std::cerr << "[-] 無法開啟檔案進行寫入" << std::endl;
        std::cerr << "    錯誤原因: " << std::strerror(errno) << std::endl;
        return 1;
    }

    return 0;
}