#include "hw5/hw5_1_2.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>

namespace hw5 {
namespace {
// Average wall time (ms) over `runs` calls of f.
template <typename F> double averageMs(int runs, F&& f) {
    using clock = std::chrono::steady_clock;
    const auto start = clock::now();
    for (int i = 0; i < runs; ++i) f();
    return std::chrono::duration<double, std::milli>(clock::now() - start).count() / runs;
}
} // namespace

// Q1-2: same pipeline with cv::dft / cv::idft; compare accuracy and execution time.
void runHw5_1_2(const Paths& paths) {
    const fs::path dir = paths.output / "hw5_1_2";
    const Image original = readRaw(paths.input / "zurich512x384.raw", 512, 384);

    constexpr int myRuns = 3, cvRuns = 50;
    Spectrum mySpectrum, cvSpectrum;
    Image myRecon, cvRecon;
    const double myDftMs = averageMs(myRuns, [&] { mySpectrum = myDft(original); });
    const double myIdftMs = averageMs(myRuns, [&] { myRecon = myIdft(mySpectrum); });
    const double cvDftMs = averageMs(cvRuns, [&] { cvSpectrum = cvDft(original); });
    const double cvIdftMs = averageMs(cvRuns, [&] { cvRecon = cvIdft(cvSpectrum); });

    double myPeak = 0;
    const Image myLog = logMagnitude(fftShift(mySpectrum), 0, &myPeak);
    const Image cvLog = logMagnitude(fftShift(cvSpectrum));
    saveImage(dir / "zurich512x384_cv_dft_spectrum", cvLog);
    saveImage(dir / "zurich512x384_cv_idft_reconstructed", cvRecon);
    // Difference of the two log spectra, amplified x1000 for visibility (all ~0 => black).
    Image diff(original.width, original.height);
    for (std::size_t i = 0; i < diff.pixels.size(); ++i)
        diff.pixels[i] = 1000.0 * std::abs(myLog.pixels[i] - cvLog.pixels[i]);
    saveImage(dir / "spectrum_difference_x1000", diff);

    double maxMag = 0;
    for (const auto& c : cvSpectrum.data) maxMag = std::max(maxMag, std::abs(c));
    const double spectrumError = maxAbsDifference(mySpectrum, cvSpectrum);
    std::ostringstream out;
    out << "item,my_implementation,opencv\n"
        << "dft_ms," << myDftMs << ',' << cvDftMs << '\n'
        << "idft_ms," << myIdftMs << ',' << cvIdftMs << '\n'
        << "total_ms," << myDftMs + myIdftMs << ',' << cvDftMs + cvIdftMs << '\n'
        << "speedup_opencv_x,," << (myDftMs + myIdftMs) / (cvDftMs + cvIdftMs) << '\n'
        << "psnr_double_dB," << psnr(original, myRecon) << ',' << psnr(original, cvRecon) << '\n'
        << "psnr_uint8_dB," << psnr(original, toByteImage(myRecon)) << ','
        << psnr(original, toByteImage(cvRecon)) << '\n'
        << "max_abs_error_double," << maxAbsDifference(original, myRecon) << ','
        << maxAbsDifference(original, cvRecon) << '\n'
        << "max_abs_spectrum_diff_my_vs_cv,," << spectrumError << '\n'
        << "relative_spectrum_diff,," << spectrumError / maxMag << '\n'
        << "max_abs_recon_diff_my_vs_cv,," << maxAbsDifference(myRecon, cvRecon) << '\n'
        << "runs_averaged," << myRuns << ',' << cvRuns << '\n';
    writeText(dir / "comparison.csv", out.str());
    std::cout << out.str();
}
} // namespace hw5
