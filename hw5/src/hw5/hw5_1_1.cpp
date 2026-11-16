#include "hw5/hw5_1_1.hpp"
#include <chrono>
#include <iostream>
#include <sstream>

namespace hw5 {
// Q1-1: own DFT -> centered, log-enhanced magnitude spectrum -> own IDFT -> PSNR check.
void runHw5_1_1(const Paths& paths) {
    using clock = std::chrono::steady_clock;
    const fs::path dir = paths.output / "hw5_1_1";
    const Image original = readRaw(paths.input / "zurich512x384.raw", 512, 384);
    saveImage(dir / "zurich512x384_original", original);

    const auto t0 = clock::now();
    const Spectrum spectrum = myDft(original);
    const auto t1 = clock::now();
    double maxImag = 0;
    const Image reconstructed = myIdft(spectrum, &maxImag);
    const auto t2 = clock::now();

    saveImage(dir / "zurich512x384_my_dft_spectrum", logMagnitude(fftShift(spectrum)));
    saveImage(dir / "zurich512x384_my_idft_reconstructed", reconstructed);

    const Image reconstructedByte = toByteImage(reconstructed);
    const double dftMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const double idftMs = std::chrono::duration<double, std::milli>(t2 - t1).count();
    std::ostringstream out;
    out << "image,zurich512x384\n"
        << "my_dft_ms," << dftMs << "\nmy_idft_ms," << idftMs << '\n'
        << "DC_F(0;0)," << spectrum.at(0, 0).real() << '\n'
        << "max_abs_imag_after_idft," << maxImag << '\n'
        << "max_abs_error_double," << maxAbsDifference(original, reconstructed) << '\n'
        << "mse_double," << mse(original, reconstructed) << '\n'
        << "psnr_double_dB," << psnr(original, reconstructed) << '\n'
        << "mse_uint8," << mse(original, reconstructedByte) << '\n'
        << "psnr_uint8_dB," << psnr(original, reconstructedByte) << '\n';
    writeText(dir / "result.csv", out.str());
    std::cout << out.str();
}
} // namespace hw5
