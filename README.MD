# 高等數位影像處理(ADIP)

## 環境
* **作業系統**：Windows 10 / 11 (使用 MSYS2 CLANG64 環境)
* **CMake**：`>= 3.15`
* **編譯器**：Clang
* **建構工具**：Ninja
* **第三方庫**：OpenCV 5.x (經 MSYS2 pacman 安裝版本為 5.0.0)
## 步驟
1. 下載msys2
2. 選擇**Clang64**終端機，執行套件更新
```bash
pacman -Syu
```
3. 下載套件
```bash
pacman -S \
  mingw-w64-clang-x86_64-toolchain \
  mingw-w64-clang-x86_64-cmake \
  mingw-w64-clang-x86_64-ninja \
  mingw-w64-clang-x86_64-opencv
```
4. 設定cmake選擇編譯器
```bash
# 1. 產生 Ninja 建構檔至 build 資料夾 (指定 Debug 模式)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# 2. 開始建置目標專案
cmake --build build --config Debug
```
## 架構
```text
homework/
├── CMakeLists.txt        # 根目錄 CMake
├── hw1/                  # 作業 1 目錄
│   ├── CMakeLists.txt    # 作業 1 CMake 設定
│   └── hw1_1_a.cpp       # 作業 1 第一題第a小題
├── hw2/                  # 作業 2 目錄
└── build/                # 編譯產出目錄