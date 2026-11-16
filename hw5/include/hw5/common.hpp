#pragma once

#include <complex>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace hw5 {
namespace fs = std::filesystem;

// Floating-point working image (row-major, width x height).
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

// Complex 2D spectrum, same layout as Image. Index (u, v) = (column, row).
struct Spectrum {
    int width = 0;
    int height = 0;
    std::vector<std::complex<double>> data;
    Spectrum() = default;
    Spectrum(int w, int h);
    std::complex<double>& at(int u, int v);
    const std::complex<double>& at(int u, int v) const;
};

struct Paths { fs::path input; fs::path output; };

// ---------- paths / IO ----------
Paths locatePaths(const fs::path& executable);
Image readRaw(const fs::path& file, int width, int height);
Image toByteImage(const Image& image);                 // clamp [0,255] + round
void saveImage(const fs::path& stem, const Image& image); // writes stem.raw + stem.png
void writeText(const fs::path& path, const std::string& text);

// ---------- metrics ----------
double mse(const Image& reference, const Image& image);
double psnr(const Image& reference, const Image& image);   // +inf when identical
double maxAbsDifference(const Image& a, const Image& b);

// ---------- display helpers ----------
Image normalizeMinMax(const Image& image);                 // linear stretch to [0,255]
// Robust linear stretch: [low, high] percentiles -> [0,255] (clipped outside).
Image normalizePercentile(const Image& image, double low = 0.01, double high = 0.99);
Image zoomNearest(const Image& image, int x0, int y0, int w, int h, int factor);

// ---------- resize (own implementation, no OpenCV) ----------
// Area-averaging (box) resampling: each output pixel = mean of the input area it covers.
Image areaResize(const Image& image, int width, int height);

// ---------- Fourier ----------
// Own separable DFT/IDFT: 1D DFT on every row, then on every column (twiddle tables precomputed).
Spectrum myDft(const Image& image);
Image myIdft(const Spectrum& spectrum, double* maxImag = nullptr);
// OpenCV cv::dft / cv::idft wrappers (DFT_COMPLEX_OUTPUT, DFT_SCALE | DFT_REAL_OUTPUT).
Spectrum cvDft(const Image& image);
Image cvIdft(const Spectrum& spectrum);
Spectrum fftShift(const Spectrum& spectrum);    // move DC to (W/2, H/2)
Spectrum ifftShift(const Spectrum& spectrum);
// log(1+|F|) scaled to [0,255]; if refMax > 0, scale by that value instead of own max.
Image logMagnitude(const Spectrum& centered, double refMax = 0.0, double* ownMax = nullptr);
double maxAbsDifference(const Spectrum& a, const Spectrum& b);

// ---------- frequency-domain filters (centered transfer functions) ----------
Image idealLowPass(int w, int h, double d0);
Image idealHighPass(int w, int h, double d0);
Image butterworthLowPass(int w, int h, double d0, int n);
Image butterworthHighPass(int w, int h, double d0, int n);
struct NotchPoint { int u; int v; };                       // offset from the spectrum center
Image butterworthNotchReject(int w, int h, const std::vector<NotchPoint>& points, double d0, int n);
// Apply centered H with OpenCV DFT; returns the filtered (centered) spectrum and spatial result.
struct FilterResult { Spectrum filteredCentered; Image spatial; };
FilterResult applyFilter(const Image& image, const Image& transfer);

// ---------- hashing ----------
std::uint64_t pHash(const Image& image, Image* dctBlock = nullptr);
std::uint64_t aHash(const Image& image, Image* small = nullptr);
int hamming(std::uint64_t a, std::uint64_t b);
std::string hashHex(std::uint64_t hash);
Image hashBitmap(std::uint64_t hash, int cell = 32);       // 8x8 bits -> (8*cell)^2 image
struct NamedImage { std::string name; Image image; };
// lena256 (query) followed by its gamma / Gaussian-noise / 9-degree-rotation versions.
std::vector<NamedImage> loadLenaSet(const Paths& paths);
// Hash table: rows = images, Hamming distance to the first image. Also saves hash bitmaps.
std::string hashReport(const std::vector<NamedImage>& images, const fs::path& dir);

// ---------- JPEG simulation ----------
using QTable = std::vector<int>;                           // 64 entries, row-major
QTable baseQuantTable();
QTable qualityTable(const QTable& base, int qf);
struct JpegResult { Image reconstructed; std::size_t nonZeroCoefficients = 0; };
JpegResult jpegSimulate(const Image& image, const QTable& table);
std::string formatTable(const QTable& table);
} // namespace hw5
