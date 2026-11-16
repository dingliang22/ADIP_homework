#include "hw5/common.hpp"
#include <cmath>

namespace hw5 {
namespace {
// Distance from the spectrum center (W/2, H/2) after fftShift.
double distance(int u, int v, int w, int h) {
    const double du = u - w / 2, dv = v - h / 2;
    return std::sqrt(du * du + dv * dv);
}
template <typename F> Image build(int w, int h, F f) {
    Image transfer(w, h);
    for (int v = 0; v < h; ++v)
        for (int u = 0; u < w; ++u) transfer.at(u, v) = f(distance(u, v, w, h), u, v);
    return transfer;
}
double butterworthHigh(double d, double d0, int n) {
    return d == 0 ? 0.0 : 1.0 / (1.0 + std::pow(d0 / d, 2 * n));
}
} // namespace

Image idealLowPass(int w, int h, double d0) {
    return build(w, h, [=](double d, int, int) { return d <= d0 ? 1.0 : 0.0; });
}
Image idealHighPass(int w, int h, double d0) {
    return build(w, h, [=](double d, int, int) { return d <= d0 ? 0.0 : 1.0; });
}
Image butterworthLowPass(int w, int h, double d0, int n) {
    return build(w, h, [=](double d, int, int) { return 1.0 / (1.0 + std::pow(d / d0, 2 * n)); });
}
Image butterworthHighPass(int w, int h, double d0, int n) {
    return build(w, h, [=](double d, int, int) { return butterworthHigh(d, d0, n); });
}
// Product of Butterworth high-pass "holes" centred at each (u_k, v_k) and its symmetric (-u_k, -v_k).
Image butterworthNotchReject(int w, int h, const std::vector<NotchPoint>& points, double d0, int n) {
    return build(w, h, [&](double, int u, int v) {
        double value = 1.0;
        const double du = u - w / 2, dv = v - h / 2;
        for (const NotchPoint& p : points) {
            const double dPlus = std::hypot(du - p.u, dv - p.v);
            const double dMinus = std::hypot(du + p.u, dv + p.v);
            value *= butterworthHigh(dPlus, d0, n) * butterworthHigh(dMinus, d0, n);
        }
        return value;
    });
}

FilterResult applyFilter(const Image& image, const Image& transfer) {
    Spectrum centered = fftShift(cvDft(image));
    for (std::size_t i = 0; i < centered.data.size(); ++i) centered.data[i] *= transfer.pixels[i];
    return {centered, cvIdft(ifftShift(centered))};
}
} // namespace hw5
