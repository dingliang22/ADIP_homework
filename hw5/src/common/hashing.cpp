#include "hw5/common.hpp"
#include <opencv2/core.hpp>
#include <bit>
#include <cstdio>
#include <numeric>
#include <sstream>

namespace hw5 {
// pHash: 32x32 area resize -> 32x32 DCT (cv::dct) -> top-left 8x8 low-frequency block.
// Mean is taken over the 63 AC coefficients (DC excluded); bit = coefficient > mean.
// Bit order: row-major over the 8x8 block, bit 63 = (0,0).
std::uint64_t pHash(const Image& image, Image* dctBlock) {
    const Image small = areaResize(image, 32, 32);
    cv::Mat input(32, 32, CV_64F, const_cast<double*>(small.pixels.data()));
    cv::Mat coefficients;
    cv::dct(input, coefficients);
    double sum = 0;
    for (int v = 0; v < 8; ++v)
        for (int u = 0; u < 8; ++u)
            if (u || v) sum += coefficients.at<double>(v, u);
    const double mean = sum / 63.0;
    std::uint64_t hash = 0;
    if (dctBlock) *dctBlock = Image(8, 8);
    for (int v = 0; v < 8; ++v)
        for (int u = 0; u < 8; ++u) {
            const double c = coefficients.at<double>(v, u);
            hash = (hash << 1) | (c > mean ? 1u : 0u);
            if (dctBlock) dctBlock->at(u, v) = c;
        }
    return hash;
}

// aHash: 8x8 area resize -> mean of 64 pixels -> bit = pixel >= mean.
std::uint64_t aHash(const Image& image, Image* smallOut) {
    const Image small = areaResize(image, 8, 8);
    const double mean = std::accumulate(small.pixels.begin(), small.pixels.end(), 0.0) / 64.0;
    std::uint64_t hash = 0;
    for (double p : small.pixels) hash = (hash << 1) | (p >= mean ? 1u : 0u);
    if (smallOut) *smallOut = small;
    return hash;
}

int hamming(std::uint64_t a, std::uint64_t b) { return std::popcount(a ^ b); }

std::string hashHex(std::uint64_t hash) {
    char buffer[17];
    std::snprintf(buffer, sizeof buffer, "%016llX", static_cast<unsigned long long>(hash));
    return buffer;
}

Image hashBitmap(std::uint64_t hash, int cell) {
    Image image(8 * cell, 8 * cell);
    for (int y = 0; y < image.height; ++y)
        for (int x = 0; x < image.width; ++x) {
            const int bit = 63 - ((y / cell) * 8 + x / cell);
            image.at(x, y) = (hash >> bit) & 1u ? 255.0 : 0.0;
        }
    return image;
}

std::vector<NamedImage> loadLenaSet(const Paths& paths) {
    std::vector<NamedImage> set;
    for (const char* name : {"lena256", "lena256_gamma_0p3", "lena256_gaussian_sigma30", "lena256_rotation_9deg"})
        set.push_back({name, readRaw(paths.input / (std::string(name) + ".raw"), 256, 256)});
    return set;
}

std::string hashReport(const std::vector<NamedImage>& images, const fs::path& dir) {
    const std::uint64_t pQuery = pHash(images.front().image), aQuery = aHash(images.front().image);
    std::ostringstream csv;
    csv << "image,pHash,pHash_hamming,aHash,aHash_hamming\n";
    for (const NamedImage& item : images) {
        const std::uint64_t p = pHash(item.image), a = aHash(item.image);
        saveImage(dir / (item.name + "_pHash_bits"), hashBitmap(p));
        saveImage(dir / (item.name + "_aHash_bits"), hashBitmap(a));
        csv << item.name << ',' << hashHex(p) << ',' << hamming(pQuery, p) << ','
            << hashHex(a) << ',' << hamming(aQuery, a) << '\n';
    }
    return csv.str();
}
} // namespace hw5
