#include "hw5/hw5_1_3.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>

namespace hw5 {
namespace {
struct Peak { NotchPoint offset; double prominence; double logMagnitude; };

// Periodic noise -> isolated bright spikes in |F|. A spike is a strict local maximum (9x9) of
// log(1+|F|) whose value exceeds the mean of its 31x31 neighbourhood by `threshold`. The
// low-frequency disc (radius rMin) and the u/v axes (boundary leakage cross) are skipped.
// Only one point of each conjugate-symmetric pair is returned.
std::vector<Peak> detectPeaks(const Spectrum& centered, double rMin, double threshold) {
    const int w = centered.width, h = centered.height, cx = w / 2, cy = h / 2;
    Image logMag(w, h);
    for (std::size_t i = 0; i < centered.data.size(); ++i) logMag.pixels[i] = std::log1p(std::abs(centered.data[i]));
    std::vector<Peak> peaks;
    for (int v = 15; v < h - 15; ++v)
        for (int u = 15; u < w - 15; ++u) {
            const int du = u - cx, dv = v - cy;
            if (std::hypot(du, dv) < rMin || std::abs(du) <= 2 || std::abs(dv) <= 2) continue;
            if (!(dv < 0 || (dv == 0 && du > 0))) continue;   // keep upper half plane only
            const double value = logMag.at(u, v);
            bool isMax = true;
            for (int y = -4; y <= 4 && isMax; ++y)
                for (int x = -4; x <= 4; ++x)
                    if ((x || y) && logMag.at(u + x, v + y) >= value) { isMax = false; break; }
            if (!isMax) continue;
            double sum = 0;
            for (int y = -15; y <= 15; ++y)
                for (int x = -15; x <= 15; ++x) sum += logMag.at(u + x, v + y);
            const double prominence = value - sum / (31.0 * 31.0);
            if (prominence > threshold) peaks.push_back({{du, dv}, prominence, value});
        }
    std::sort(peaks.begin(), peaks.end(), [](const Peak& a, const Peak& b) { return a.prominence > b.prominence; });
    return peaks;
}

double powerRatio(const Spectrum& filtered, const Spectrum& original) {
    double a = 0, b = 0;
    for (std::size_t i = 0; i < filtered.data.size(); ++i) {
        a += std::norm(filtered.data[i]);
        b += std::norm(original.data[i]);
    }
    return a / b;
}
} // namespace

// Q1-3: ILPF / IHPF / BLPF / BHPF on zurich512x384 and a Butterworth notch reject filter on the
// noisy version, all using OpenCV DFT/IDFT. Saves transfer function, filtered spectrum, result.
void runHw5_1_3(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_1_3";
    const Image image = readRaw(paths.input / "zurich512x384.raw", 512, 384);
    const int w = image.width, h = image.height;
    const Spectrum spectrum = fftShift(cvDft(image));
    double refMax = 0;
    saveImage(dir / "zurich_spectrum", logMagnitude(spectrum, 0, &refMax));

    struct Job { std::string name; Image transfer; bool highPass; };
    std::vector<Job> jobs;
    for (int d0 : {10, 100}) jobs.push_back({"ILPF_D" + std::to_string(d0), idealLowPass(w, h, d0), false});
    for (int d0 : {10, 100}) jobs.push_back({"IHPF_D" + std::to_string(d0), idealHighPass(w, h, d0), true});
    for (int d0 : {10, 100})
        for (int n : {1, 3})
            jobs.push_back({"BLPF_D" + std::to_string(d0) + "_n" + std::to_string(n),
                            butterworthLowPass(w, h, d0, n), false});
    for (int d0 : {10, 100})
        for (int n : {1, 3})
            jobs.push_back({"BHPF_D" + std::to_string(d0) + "_n" + std::to_string(n),
                            butterworthHighPass(w, h, d0, n), true});

    std::ostringstream csv;
    csv << "filter,power_retained_percent,psnr_vs_original_dB,result_min,result_max\n";
    for (const Job& job : jobs) {
        const FilterResult r = applyFilter(image, job.transfer);
        Image mask = job.transfer;
        for (double& p : mask.pixels) p *= 255.0;
        saveImage(dir / (job.name + "_H"), mask);
        saveImage(dir / (job.name + "_spectrum"), logMagnitude(r.filteredCentered, refMax));
        // High-pass output is zero-mean with negative values and a few large outliers at strong
        // edges -> 1%-99% percentile stretch for display (min-max would leave it almost flat gray).
        saveImage(dir / (job.name + "_result"), job.highPass ? normalizePercentile(r.spatial) : r.spatial);
        const auto [lo, hi] = std::minmax_element(r.spatial.pixels.begin(), r.spatial.pixels.end());
        csv << job.name << ',' << 100.0 * powerRatio(r.filteredCentered, spectrum) << ','
            << (job.highPass ? std::string("-") : std::to_string(psnr(image, toByteImage(r.spatial))))
            << ',' << *lo << ',' << *hi << '\n';
    }

    // ---- Notch reject on zurich512x384_noise ----
    const Image noisy = readRaw(paths.input / "zurich512x384_noise.raw", 512, 384);
    saveImage(dir / "zurich512x384_noise", noisy);
    const Spectrum noisySpectrum = fftShift(cvDft(noisy));
    double noisyMax = 0;
    saveImage(dir / "noise_spectrum", logMagnitude(noisySpectrum, 0, &noisyMax));
    constexpr double rMin = 20.0, threshold = 2.0, notchD0 = 6.0;
    constexpr int notchOrder = 4;
    const auto peaks = detectPeaks(noisySpectrum, rMin, threshold);
    std::vector<NotchPoint> points;
    std::ostringstream peakText;
    peakText << "du,dv,prominence_log,log_magnitude\n";
    for (const Peak& p : peaks) {
        points.push_back(p.offset);
        peakText << p.offset.u << ',' << p.offset.v << ',' << p.prominence << ',' << p.logMagnitude << '\n';
    }
    writeText(dir / "notch_peaks.csv", peakText.str());

    const Image notch = butterworthNotchReject(w, h, points, notchD0, notchOrder);
    const FilterResult r = applyFilter(noisy, notch);
    Image mask = notch;
    for (double& p : mask.pixels) p *= 255.0;
    saveImage(dir / "Notch_H", mask);
    saveImage(dir / "Notch_spectrum", logMagnitude(r.filteredCentered, noisyMax));
    saveImage(dir / "Notch_result", r.spatial);
    // Noise pattern removed = noisy - restored, centred at 128.
    Image removed(w, h);
    for (std::size_t i = 0; i < removed.pixels.size(); ++i) removed.pixels[i] = noisy.pixels[i] - r.spatial.pixels[i];
    saveImage(dir / "Notch_removed_noise", normalizeMinMax(removed));

    const Image restored = toByteImage(r.spatial);
    std::ostringstream notchText;
    notchText << "detected_pairs," << points.size() << "\nrMin," << rMin << "\nthreshold_log," << threshold
              << "\nnotch_D0," << notchD0 << "\nnotch_order," << notchOrder
              << "\npsnr_noisy_vs_clean_dB," << psnr(image, noisy)
              << "\npsnr_restored_vs_clean_dB," << psnr(image, restored) << '\n';
    writeText(dir / "notch_result.csv", notchText.str());
    writeText(dir / "filters.csv", csv.str());
    std::cout << csv.str() << peakText.str() << notchText.str();
}
} // namespace hw5
