#include "hw4/common.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace hw4 {
namespace {
void checkPair(const Image& a, const Image& b) {
    if (!a.valid() || !b.valid() || a.width != b.width || a.height != b.height)
        throw std::invalid_argument("Image dimensions differ");
}
// Lower envelope of parabolas: exact squared Euclidean distance transform, O(n).
std::vector<double> edt1d(const std::vector<double>& f) {
    const int n = static_cast<int>(f.size());
    std::vector<int> v(n);
    std::vector<double> z(n + 1), result(n, 1e20);
    int k = -1;
    for (int q = 0; q < n; ++q) {
        if (f[q] >= 1e20) continue;
        double intersection = -std::numeric_limits<double>::infinity();
        while (k >= 0) {
            const double p = v[k];
            intersection = ((f[q] + static_cast<double>(q)*q) - (f[v[k]] + p*p)) / (2*(q-p));
            if (intersection > z[k]) break;
            --k;
        }
        ++k;
        v[k] = q;
        z[k] = k == 0 ? -std::numeric_limits<double>::infinity() : intersection;
        z[k + 1] = std::numeric_limits<double>::infinity();
    }
    if (k < 0) return result;
    int envelope = 0;
    for (int q = 0; q < n; ++q) {
        while (z[envelope + 1] < q) ++envelope;
        const double delta = q - v[envelope];
        result[q] = delta*delta + f[v[envelope]];
    }
    return result;
}
}
double psnr(const Image& reference, const Image& image) {
    checkPair(reference, image);
    double sum = 0;
    for (std::size_t i = 0; i < image.pixels.size(); ++i) {
        const double difference = reference.pixels[i] - image.pixels[i];
        sum += difference*difference;
    }
    if (sum == 0) return std::numeric_limits<double>::infinity();
    return 10 * std::log10(255.0 * 255.0 * image.pixels.size() / sum);
}
double meanAbsoluteDifference(const Image& a, const Image& b) {
    checkPair(a,b);
    double sum = 0;
    for (std::size_t i = 0; i < a.pixels.size(); ++i) sum += std::abs(a.pixels[i] - b.pixels[i]);
    return sum / a.pixels.size();
}
std::size_t differentPixels(const Image& a, const Image& b) {
    checkPair(a,b);
    std::size_t count = 0;
    for (std::size_t i = 0; i < a.pixels.size(); ++i)
        if (std::abs(a.pixels[i] - b.pixels[i]) > 1e-9) ++count;
    return count;
}
double prattFom(const Image& groundTruth, const Image& detected, double alpha) {
    checkPair(groundTruth, detected);
    if (!(alpha > 0) || !std::isfinite(alpha)) throw std::invalid_argument("Invalid FOM alpha");
    const auto countEdges = [](const Image& image) {
        return std::count_if(image.pixels.begin(), image.pixels.end(), [](double p){return p > 0;});
    };
    const auto idealCount = countEdges(groundTruth), detectedCount = countEdges(detected);
    // Empty-set convention is explicit; avoids undefined normalization or distances.
    if (!idealCount || !detectedCount) return idealCount == detectedCount ? 1.0 : 0.0;
    Image distance(groundTruth.width, groundTruth.height);
    for (int y = 0; y < groundTruth.height; ++y) {
        std::vector<double> row(groundTruth.width);
        for (int x = 0; x < groundTruth.width; ++x) row[x] = groundTruth.at(x,y) > 0 ? 0 : 1e20;
        const auto transformed = edt1d(row);
        for (int x = 0; x < groundTruth.width; ++x) distance.at(x,y) = transformed[x];
    }
    for (int x = 0; x < groundTruth.width; ++x) {
        std::vector<double> column(groundTruth.height);
        for (int y = 0; y < groundTruth.height; ++y) column[y] = distance.at(x,y);
        const auto transformed = edt1d(column);
        for (int y = 0; y < groundTruth.height; ++y) distance.at(x,y) = transformed[y];
    }
    double score = 0;
    for (std::size_t i = 0; i < detected.pixels.size(); ++i)
        if (detected.pixels[i] > 0) score += 1.0 / (1.0 + alpha * distance.pixels[i]);
    return score / static_cast<double>(std::max(idealCount, detectedCount));
}
} // namespace hw4
