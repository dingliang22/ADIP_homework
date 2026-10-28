#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace hw4 {
namespace fs = std::filesystem;

// Floating-point working image: retain signed responses and fractional corrections.
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
using Images = std::map<std::string, Image>;
struct EdgeResult { std::string name; Image response; Image strength; };

Paths locatePaths(const fs::path& executable, const fs::path& inputOverride = {},
                  const fs::path& outputOverride = {});
Image readRaw(const fs::path& file, int width, int height);
Image toByteImage(const Image& image);
void saveImage(const fs::path& stem, const Image& image);
void writeText(const fs::path& path, const std::string& text);
double psnr(const Image& reference, const Image& image);
double meanAbsoluteDifference(const Image& a, const Image& b);
std::size_t differentPixels(const Image& a, const Image& b);

Image convolve(const Image& image, const std::vector<double>& kernel, int size);
Image meanFilter(const Image& image, int size);
Image gaussianFilter(const Image& image, int size, double sigma);
Image medianFilter(const Image& image, int size);
Image nearestResize(const Image& image, int width, int height);
Image bilinearResize(const Image& image, int width, int height);
Image decimate2(const Image& image);
Images ibpSnapshots(const Image& lowResolution);

// Magnitudes for Laplacian/Sobel; zero-crossing contrast for LoG.
EdgeResult laplacian(const Image& image, bool eightNeighbors);
EdgeResult sobel(const Image& image, int direction); // 0: Gx, 90: Gy, -1: magnitude
EdgeResult logEdges(const Image& image);
Image signedPreview(const Image& response);
Image magnitudePreview(const Image& strength);
Image binarize(const Image& strength, double threshold);
double percentile(const Image& image, double fraction);
double prattFom(const Image& groundTruth, const Image& detected, double alpha = 1.0 / 9.0);

Images denoiseSingles(const Image& noisy);
Images denoiseMedians(const Image& noisy);
Images denoiseCascades(const Image& noisy);
std::vector<EdgeResult> standardEdges(const Image& image);
void saveEdge(const fs::path& dir, const std::string& prefix, const EdgeResult& edge);
void saveImages(const fs::path& dir, const Images& images);
void writeParameters(const Paths& paths);
} // namespace hw4
