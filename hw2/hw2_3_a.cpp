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

vector<int> findShortestPathD4(const vector<unsigned char>& image) {
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
    constexpr array<int, 4> dx = {1, -1, 0, 0};
    constexpr array<int, 4> dy = {0, 0, 1, -1};
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        if (current == endIndex) {
            break;
        }
        int x = current % WIDTH;
        int y = current / WIDTH;
        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (!inBounds(nx, ny)) {
                continue;
            }
            if (!isRoad(image, nx, ny)) {
                continue;
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
    const string output_path = current_dir + "/hw2_img_result/map_D4.png";
    vector<unsigned char> image;

    if (!loadRawImage(input_path, image)) {
        return -1;
    }
    vector<int> path = findShortestPathD4(image);
    if (path.empty()) {
        cerr << "找不到從 "
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
    cout << "\n"
         << "============================\n"
         << "D4 Shortest Path Result\n"
         << "============================\n"
         << "Path length: " << steps << " steps\n"
         << "Output:\n"
         << output_path << '\n';
    return 0;
}