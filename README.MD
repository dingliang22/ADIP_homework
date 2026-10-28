# 高等數位影像處理(ADIP)

## 環境
* **作業系統**：Windows 10 / 11 (使用 MSYS2 CLANG64 環境)
* **CMake**：`>= 3.18`
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
  mingw-w64-clang-x86_64-opencv \
  mingw-w64-clang-x86_64-stb
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
ADIP_homework/
├── CMakeLists.txt        # 根目錄建置設定
├── hw1/                  # 各小題獨立執行檔
├── hw2/                  # 各小題獨立執行檔、hw2_img 輸入資料
├── hw3/                  # main.cpp + include/hw3 + src/common + src/hw3
├── hw4/                  # main.cpp + include/hw4 + src/common + src/hw4
│   ├── hw4_img/          # 六張輸入 RAW
│   ├── hw4_img_result/   # 圖片與 CSV 輸出（不納入版本控制）
│   └── README.md         # 題目、參數、選單與路徑說明
└── build/                # 編譯產出
```

