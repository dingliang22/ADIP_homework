#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace hw6 {
namespace fs = std::filesystem;

// Floating-point working image (row-major). Binary images use 0 / 255.
struct Image {
    int width = 0;
    int height = 0;
    std::vector<double> pixels;
    Image() = default;
    Image(int w, int h, double value = 0.0);
    bool valid() const;
    double& at(int x, int y);
    double at(int x, int y) const;
};

struct Paths { fs::path input; fs::path output; };

// ---------- paths / IO ----------
Paths locatePaths(const fs::path& executable);
Image readRaw(const fs::path& file, int width, int height);
Image toByteImage(const Image& image);                     // clamp [0,255] + round
void saveImage(const fs::path& stem, const Image& image);  // stem.raw + stem.png
// RGB PNG (r, g, b planes of equal size), used to mark minutiae.
void saveRgbPng(const fs::path& file, const Image& r, const Image& g, const Image& b);
void writeText(const fs::path& path, const std::string& text);
Image normalizeMinMax(const Image& image);                 // linear stretch to [0,255]

// ---------- structuring elements ----------
struct Offset { int dx; int dy; };
using StructuringElement = std::vector<Offset>;
StructuringElement squareSE(int radius);                   // (2r+1) x (2r+1)
StructuringElement diskSE(int radius);                     // dx^2 + dy^2 <= r^2

// ---------- grayscale morphology (binary = special case on 0/255) ----------
// Pixels outside the image are ignored (erosion treats them as +inf, dilation as -inf).
Image erode(const Image& image, const StructuringElement& se);
Image dilate(const Image& image, const StructuringElement& se);
Image opening(const Image& image, const StructuringElement& se);   // erode -> dilate
Image closing(const Image& image, const StructuringElement& se);   // dilate -> erode
Image whiteTopHat(const Image& image, const StructuringElement& se); // f - open(f)
Image blackTopHat(const Image& image, const StructuringElement& se); // close(f) - f
// Morphological reconstruction by dilation of `marker` under `mask` (marker <= mask),
// 8-connected, Vincent's hybrid raster / anti-raster + FIFO algorithm.
Image reconstructByDilation(const Image& marker, const Image& mask);
// Opening by reconstruction: erode with se, then reconstruct under the original image.
Image openingByReconstruction(const Image& image, const StructuringElement& se);

// ---------- binary helpers ----------
Image invert(const Image& image);                          // 255 - p
Image removeSmallComponents(const Image& binary, int minArea); // area opening (8-connected)

// ---------- thinning / minutiae ----------
// Zhang-Suen thinning of the foreground (value 255) -> one-pixel-wide skeleton.
Image zhangSuenThinning(const Image& binary, int* iterations = nullptr);
// Crossing number of a foreground pixel: 0.5 * sum |P_i - P_{i+1}| over the 8 clockwise neighbours.
int crossingNumber(const Image& skeleton, int x, int y);
struct Minutia { int x; int y; int cn; };
std::vector<Minutia> detectMinutiae(const Image& skeleton);   // CN == 1 or CN == 3
} // namespace hw6
