#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;
constexpr int ORIG_W = 512;
constexpr int ORIG_H = 512;
constexpr int TARGET_W = 1024;
constexpr int TARGET_H = 1024;
constexpr size_t ORIG_SIZE = static_cast<size_t>(ORIG_W) * ORIG_H;
constexpr size_t TARGET_SIZE = static_cast<size_t>(TARGET_W) * TARGET_H;

enum Method {
    NEAREST = 0,
    BILINEAR = 1,
    MULTI_FRAME_SR = 2,
    METHOD_COUNT = 3,
};

bool loadRaw(const string& filepath, vector<unsigned char>& image, size_t expectedSize) {
    ifstream file(filepath, ios::binary);
    if (!file) {
        cerr << "無法開啟檔案: " << filepath << '\n';
        return false;
    }
    image.resize(expectedSize);
    file.read(reinterpret_cast<char*>(image.data()), static_cast<streamsize>(expectedSize));
    if (file.gcount() != static_cast<streamsize>(expectedSize)) {
        cerr << "檔案大小不正確: " << filepath << '\n';
        cerr << "預期: " << expectedSize << " bytes\n";
        cerr << "實際讀取: " << file.gcount() << " bytes\n";
        return false;
    }
    return true;
}

void nearestNeighborResize(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    const float scaleX = static_cast<float>(ORIG_W) / TARGET_W;
    const float scaleY = static_cast<float>(ORIG_H) / TARGET_H;
    for (int y = 0; y < TARGET_H; ++y) {
        for (int x = 0; x < TARGET_W; ++x) {
            int srcX = static_cast<int>(round(x * scaleX));
            int srcY = static_cast<int>(round(y * scaleY));
            srcX = min(srcX, ORIG_W - 1);
            srcY = min(srcY, ORIG_H - 1);
            dst[static_cast<size_t>(y) * TARGET_W + x] =
                src[static_cast<size_t>(srcY) * ORIG_W + srcX];
        }
    }
}

void bilinearResize(const vector<unsigned char>& src, vector<unsigned char>& dst) {
    const float scaleX = static_cast<float>(ORIG_W) / TARGET_W;
    const float scaleY = static_cast<float>(ORIG_H) / TARGET_H;
    for (int y = 0; y < TARGET_H; ++y) {
        for (int x = 0; x < TARGET_W; ++x) {
            const float srcX = x * scaleX;
            const float srcY = y * scaleY;
            int x1 = static_cast<int>(floor(srcX));
            int y1 = static_cast<int>(floor(srcY));
            x1 = min(x1, ORIG_W - 1);
            y1 = min(y1, ORIG_H - 1);
            const int x2 = min(x1 + 1, ORIG_W - 1);
            const int y2 = min(y1 + 1, ORIG_H - 1);
            const float dx = srcX - x1;
            const float dy = srcY - y1;
            const float p1 = src[static_cast<size_t>(y1) * ORIG_W + x1];
            const float p2 = src[static_cast<size_t>(y1) * ORIG_W + x2];
            const float p3 = src[static_cast<size_t>(y2) * ORIG_W + x1];
            const float p4 = src[static_cast<size_t>(y2) * ORIG_W + x2];
            const float interp = (1.0f - dx) * (1.0f - dy) * p1 + dx * (1.0f - dy) * p2 +
                                 (1.0f - dx) * dy * p3 + dx * dy * p4;
            dst[static_cast<size_t>(y) * TARGET_W + x] = static_cast<unsigned char>(round(interp));
        }
    }
}

void multiFrameSuperResolution(const array<vector<unsigned char>, 4>& lrImages,
                               vector<unsigned char>& dst) {
    const array<int, 4> dx = {0, 1, 0, 1};
    const array<int, 4> dy = {0, 0, 1, 1};
    for (int Y = 0; Y < TARGET_H; ++Y) {
        for (int X = 0; X < TARGET_W; ++X) {
            int sum = 0;
            int count = 0;
            for (size_t k = 0; k < lrImages.size(); ++k) {
                const int x = static_cast<int>(floor((X - dx[k]) / 2.0));
                const int y = static_cast<int>(floor((Y - dy[k]) / 2.0));
                if (x >= 0 && x < ORIG_W && y >= 0 && y < ORIG_H) {
                    const size_t lrIndex = static_cast<size_t>(y) * ORIG_W + x;
                    sum += lrImages[k][lrIndex];
                    ++count;
                }
            }
            if (count > 0) {
                dst[static_cast<size_t>(Y) * TARGET_W + X] =
                    static_cast<unsigned char>(sum / count);
            }
        }
    }
}

double calculateMSE(const vector<unsigned char>& reference,
                    const vector<unsigned char>& reconstructed) {
    if (reference.size() != reconstructed.size()) {
        return numeric_limits<double>::quiet_NaN();
    }
    double squaredErrorSum = 0.0;
    for (size_t i = 0; i < reference.size(); ++i) {
        const double difference =
            static_cast<double>(reference[i]) - static_cast<double>(reconstructed[i]);
        squaredErrorSum += difference * difference;
    }
    return squaredErrorSum / static_cast<double>(reference.size());
}

double calculatePSNR(const vector<unsigned char>& reference,
                     const vector<unsigned char>& reconstructed) {
    const double mse = calculateMSE(reference, reconstructed);
    if (mse == 0.0) {
        return numeric_limits<double>::infinity();
    }
    constexpr double MAX_PIXEL_VALUE = 255.0;
    return 10.0 * log10((MAX_PIXEL_VALUE * MAX_PIXEL_VALUE) / mse);
}

int main() {
    const string currentDir = filesystem::path(__FILE__).parent_path().string();
    const string referencePath = currentDir + "/hw2_img/lena1024.raw";
    const array<string, 4> lrPaths = {
        currentDir + "/hw2_img/lena512_0.raw",
        currentDir + "/hw2_img/lena512_1.raw",
        currentDir + "/hw2_img/lena512_2.raw",
        currentDir + "/hw2_img/lena512_3.raw",
    };
    vector<unsigned char> referenceImage;
    if (!loadRaw(referencePath, referenceImage, TARGET_SIZE)) {
        return -1;
    }
    array<vector<unsigned char>, 4> lrImages;
    for (size_t i = 0; i < lrImages.size(); ++i) {
        if (!loadRaw(lrPaths[i], lrImages[i], ORIG_SIZE)) {
            return -1;
        }
    }
    array<vector<unsigned char>, METHOD_COUNT> results;
    for (auto& image : results) {
        image.resize(TARGET_SIZE);
    }
    nearestNeighborResize(lrImages[0], results[NEAREST]);
    bilinearResize(lrImages[0], results[BILINEAR]);
    multiFrameSuperResolution(lrImages, results[MULTI_FRAME_SR]);
    // 計算MSE和PSNR
    array<double, METHOD_COUNT> mseResults;
    array<double, METHOD_COUNT> psnrResults;
    for (size_t i = 0; i < METHOD_COUNT; ++i) {
        mseResults[i] = calculateMSE(referenceImage, results[i]);
        psnrResults[i] = calculatePSNR(referenceImage, results[i]);
    }
    const array<string, METHOD_COUNT> methodNames = {"Nearest Neighbor", "Bilinear Interpolation",
                                                     "Multi-frame Super-resolution"};
    cout << fixed << setprecision(4);
    cout << "\n"
         << "=============================================================\n"
         << "             PSNR Reconstruction Comparison\n"
         << "=============================================================\n";
    cout << left << setw(32) << "Method" << right << setw(15) << "MSE" << setw(15) << "PSNR (dB)"
         << '\n';
    cout << "-------------------------------------------------------------\n";
    for (size_t i = 0; i < METHOD_COUNT; ++i) {
        cout << left << setw(32) << methodNames[i] << right << setw(15) << mseResults[i];
        if (isinf(psnrResults[i])) {
            cout << setw(15) << "INF";
        } else {
            cout << setw(15) << psnrResults[i];
        }
        cout << '\n';
    }
    cout << "=============================================================\n";
    return 0;
}