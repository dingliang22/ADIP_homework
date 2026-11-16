#include "hw5/common.hpp"
#include <opencv2/core.hpp>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace hw5 {
namespace {
using cd = std::complex<double>;

// Twiddle table W_N^m = exp(sign * j*2*pi*m/N), m = 0..N-1. Index (k*n) mod N avoids recomputing
// sin/cos inside the O(N^2) loop.
std::vector<cd> twiddles(int n, double sign) {
    std::vector<cd> table(n);
    for (int m = 0; m < n; ++m) {
        const double angle = sign * 2.0 * std::numbers::pi * m / n;
        table[m] = {std::cos(angle), std::sin(angle)};
    }
    return table;
}

// Direct 1D DFT of `count` samples read from `in` with stride, written to `out` with stride.
void dft1d(const cd* in, cd* out, int n, std::size_t stride, const std::vector<cd>& table) {
    for (int k = 0; k < n; ++k) {
        cd sum = 0;
        std::size_t index = 0;   // (k * m) mod n, updated incrementally
        for (int m = 0; m < n; ++m) {
            sum += in[m * stride] * table[index];
            index += k;
            if (index >= static_cast<std::size_t>(n)) index -= n;
        }
        out[k * stride] = sum;
    }
}

// Separable 2D DFT: rows then columns. sign = -1 forward, +1 inverse (unscaled).
Spectrum transform2d(Spectrum s, double sign) {
    const int w = s.width, h = s.height;
    const auto rowTable = twiddles(w, sign), colTable = twiddles(h, sign);
    std::vector<cd> buffer(std::max(w, h));
    for (int v = 0; v < h; ++v) {
        cd* row = &s.data[static_cast<std::size_t>(v) * w];
        dft1d(row, buffer.data(), w, 1, rowTable);
        std::copy(buffer.begin(), buffer.begin() + w, row);
    }
    std::vector<cd> column(h);
    for (int u = 0; u < w; ++u) {
        for (int v = 0; v < h; ++v) column[v] = s.at(u, v);
        dft1d(column.data(), buffer.data(), h, 1, colTable);
        for (int v = 0; v < h; ++v) s.at(u, v) = buffer[v];
    }
    return s;
}

Spectrum shiftBy(const Spectrum& s, int dx, int dy) {
    Spectrum out(s.width, s.height);
    for (int v = 0; v < s.height; ++v)
        for (int u = 0; u < s.width; ++u)
            out.at((u + dx) % s.width, (v + dy) % s.height) = s.at(u, v);
    return out;
}
} // namespace

Spectrum myDft(const Image& image) {
    Spectrum s(image.width, image.height);
    for (std::size_t i = 0; i < image.pixels.size(); ++i) s.data[i] = image.pixels[i];
    return transform2d(std::move(s), -1.0);
}

Image myIdft(const Spectrum& spectrum, double* maxImag) {
    const Spectrum s = transform2d(spectrum, +1.0);
    const double scale = 1.0 / (static_cast<double>(s.width) * s.height);
    Image image(s.width, s.height);
    double imag = 0;
    for (std::size_t i = 0; i < s.data.size(); ++i) {
        image.pixels[i] = s.data[i].real() * scale;
        imag = std::max(imag, std::abs(s.data[i].imag() * scale));
    }
    if (maxImag) *maxImag = imag;
    return image;
}

Spectrum cvDft(const Image& image) {
    cv::Mat input(image.height, image.width, CV_64F, const_cast<double*>(image.pixels.data()));
    cv::Mat output;
    cv::dft(input, output, cv::DFT_COMPLEX_OUTPUT);
    Spectrum s(image.width, image.height);
    for (int v = 0; v < image.height; ++v)
        for (int u = 0; u < image.width; ++u) {
            const auto c = output.at<cv::Vec2d>(v, u);
            s.at(u, v) = {c[0], c[1]};
        }
    return s;
}

Image cvIdft(const Spectrum& spectrum) {
    cv::Mat input(spectrum.height, spectrum.width, CV_64FC2);
    for (int v = 0; v < spectrum.height; ++v)
        for (int u = 0; u < spectrum.width; ++u) {
            const auto c = spectrum.at(u, v);
            input.at<cv::Vec2d>(v, u) = {c.real(), c.imag()};
        }
    cv::Mat output;
    cv::idft(input, output, cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
    Image image(spectrum.width, spectrum.height);
    for (int y = 0; y < image.height; ++y)
        for (int x = 0; x < image.width; ++x) image.at(x, y) = output.at<double>(y, x);
    return image;
}

Spectrum fftShift(const Spectrum& s) { return shiftBy(s, s.width / 2, s.height / 2); }
Spectrum ifftShift(const Spectrum& s) { return shiftBy(s, (s.width + 1) / 2, (s.height + 1) / 2); }

Image logMagnitude(const Spectrum& centered, double refMax, double* ownMax) {
    Image image(centered.width, centered.height);
    double peak = 0;
    for (std::size_t i = 0; i < centered.data.size(); ++i) {
        image.pixels[i] = std::log1p(std::abs(centered.data[i]));
        peak = std::max(peak, image.pixels[i]);
    }
    if (ownMax) *ownMax = peak;
    const double scale = refMax > 0 ? refMax : peak;
    for (double& p : image.pixels) p = scale > 0 ? 255.0 * p / scale : 0.0;
    return image;
}

double maxAbsDifference(const Spectrum& a, const Spectrum& b) {
    if (a.width != b.width || a.height != b.height) throw std::invalid_argument("Spectrum size differs");
    double m = 0;
    for (std::size_t i = 0; i < a.data.size(); ++i) m = std::max(m, std::abs(a.data[i] - b.data[i]));
    return m;
}
} // namespace hw5
