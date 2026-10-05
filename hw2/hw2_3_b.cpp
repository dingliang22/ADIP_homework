#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <stb/stb_image_write.h>
#include <string>
#include <vector>
using namespace std;
constexpr int WIDTH = 24;
constexpr int HEIGHT = 24;
constexpr int START_X = 0;
constexpr int START_Y = 0;
constexpr int END_X = 23;
constexpr int END_Y = 23;
constexpr unsigned char ROAD_VALUE = 72;
constexpr unsigned char PATH_VALUE = 255;
constexpr size_t IMAGE_SIZE = static_cast<size_t>(WIDTH) * HEIGHT;

int toIndex(int x, int y) {
    return y * WIDTH + x;
}

bool inBounds(int x, int y) {
    return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
}

bool loadRawImage(const string& filepath, vector<unsigned char>& image) {
    ifstream file(filepath, ios::binary);
    if (!file) {
        cerr << "Error: 無法開啟 RAW 檔案:\n" << filepath << '\n';
        return false;
    }
    image.resize(IMAGE_SIZE);
    file.read(reinterpret_cast<char*>(image.data()), static_cast<streamsize>(IMAGE_SIZE));
    if (file.gcount() != static_cast<streamsize>(IMAGE_SIZE)) {
        cerr << "Error: RAW 檔案大小不正確\n"
             << "預期: " << IMAGE_SIZE << " bytes\n"
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

bool isRoad(const vector<unsigned char>& image, int x, int y) {
    if (!inBounds(x, y)) {
        return false;
    }
    return image[toIndex(x, y)] == ROAD_VALUE;
}

// m-adjacency 的 diagonal 判斷
bool isValidMDiagonal(const vector<unsigned char>& image, int x, int y, int nx, int ny) {
    int dx = nx - x;
    int dy = ny - y;
    // 必須真的是對角線
    if ((dx != 1 && dx != -1) || (dy != 1 && dy != -1)) {
        return false;
    }
    // p 與 q 共同的兩個 4-neighbors
    int common1X = x + dx;
    int common1Y = y;
    int common2X = x;
    int common2Y = y + dy;
    bool common1IsRoad = isRoad(image, common1X, common1Y);
    bool common2IsRoad = isRoad(image, common2X, common2Y);
    // 若共同的 N4 中沒有道路 pixel，
    // 才允許 diagonal m-adjacency
    return !common1IsRoad && !common2IsRoad;
}

vector<int> findShortestPathDm(const vector<unsigned char>& image) {
    const int startIndex = toIndex(START_X, START_Y);
    const int endIndex = toIndex(END_X, END_Y);
    if (image[startIndex] != ROAD_VALUE) {
        cerr << "Error: 起點 (0,0) 不是道路灰值 72\n";
        return {};
    }
    if (image[endIndex] != ROAD_VALUE) {
        cerr << "Error: 終點 (23,23) 不是道路灰值 72\n";
        return {};
    }
    vector<bool> visited(IMAGE_SIZE, false);
    vector<int> parent(IMAGE_SIZE, -1);
    queue<int> q;
    visited[startIndex] = true;
    q.push(startIndex);
    // 前四個是 4-neighbors
    // 後四個是 diagonal neighbors
    constexpr array<int, 8> dx = {1, -1, 0, 0, 1, 1, -1, -1};
    constexpr array<int, 8> dy = {0, 0, 1, -1, 1, -1, 1, -1};
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        if (current == endIndex) {
            break;
        }
        int x = current % WIDTH;
        int y = current / WIDTH;
        for (int dir = 0; dir < 8; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (!inBounds(nx, ny)) {
                continue;
            }
            if (!isRoad(image, nx, ny)) {
                continue;
            }
            // diagonal neighbor
            if (dir >= 4) {
                if (!isValidMDiagonal(image, x, y, nx, ny)) {
                    continue;
                }
            }
            int nextIndex = toIndex(nx, ny);
            if (visited[nextIndex]) {
                continue;
            }
            visited[nextIndex] = true;
            parent[nextIndex] = current;
            q.push(nextIndex);
        }
    }
    if (!visited[endIndex]) {
        return {};
    }
    vector<int> path;
    int current = endIndex;
    while (current != -1) {
        path.push_back(current);
        if (current == startIndex) {
            break;
        }
        current = parent[current];
    }
    reverse(path.begin(), path.end());
    return path;
}

void markPathWhite(vector<unsigned char>& image, const vector<int>& path) {
    for (int index : path) {
        image[index] = PATH_VALUE;
    }
}

int main() {
    const string current_dir = filesystem::path(__FILE__).parent_path().string();
    const string input_path = current_dir + "/hw2_img/map24x24.raw";
    const string output_path = current_dir + "/hw2_img_result/map_Dm.png";
    vector<unsigned char> image;
    if (!loadRawImage(input_path, image)) {
        return -1;
    }
    vector<int> path = findShortestPathDm(image);
    if (path.empty()) {
        cerr << "Error: Dm 找不到從 "
             << "(0,0) 到 (23,23)"
             << " 的合法路徑\n";
        return -1;
    }
    vector<unsigned char> result = image;
    markPathWhite(result, path);
    if (!saveGrayPNG(output_path, result)) {
        return -1;
    }
    int steps = static_cast<int>(path.size()) - 1;
    cout << "Path length: " << steps << " steps\n";
    return 0;
}