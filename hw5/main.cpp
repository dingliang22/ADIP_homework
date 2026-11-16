#include "hw5/common.hpp"
#include "hw5/hw5_1_1.hpp"
#include "hw5/hw5_1_2.hpp"
#include "hw5/hw5_1_3.hpp"
#include "hw5/hw5_2_2.hpp"
#include "hw5/hw5_2_3.hpp"
#include "hw5/hw5_2_4.hpp"
#include "hw5/hw5_3_1.hpp"
#include "hw5/hw5_3_2.hpp"
#include "hw5/hw5_3_3.hpp"
#include "hw5/hw5_3_4.hpp"

#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {
    struct Task {
        const char* id;
        const char* title;
        void (*run)(const hw5::Paths&);
    };

    const std::array<Task, 10> tasks{
        {{"1-1", "Own DFT / IDFT, spectrum, PSNR", hw5::runHw5_1_1},
         {"1-2", "OpenCV DFT / IDFT, accuracy and timing", hw5::runHw5_1_2},
         {"1-3", "ILPF / IHPF / BLPF / BHPF / Notch reject", hw5::runHw5_1_3},
         {"2-2", "pHash + Hamming distance", hw5::runHw5_2_2},
         {"2-3", "aHash + Hamming distance", hw5::runHw5_2_3},
         {"2-4", "pHash vs aHash table", hw5::runHw5_2_4},
         {"3-1", "JPEG simulation with table Q", hw5::runHw5_3_1},
         {"3-2", "JPEG simulation QF = 20 / 80", hw5::runHw5_3_2},
         {"3-3", "JPEG MSE / PSNR / artifacts", hw5::runHw5_3_3},
         {"3-4", "JPEG pHash / aHash distances", hw5::runHw5_3_4}}};

    void runTask(std::size_t index, const hw5::Paths& paths) {
        std::cout << "\n[" << tasks[index].id << "] " << tasks[index].title << '\n';
        tasks[index].run(paths);
        std::cout << "Completed " << tasks[index].id << '\n';
    }

    int resolveTask(const std::string& choice) {
        for (std::size_t i = 0; i < tasks.size(); ++i) {
            if (choice == tasks[i].id || choice == std::to_string(i + 1)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void dispatch(const std::string& choice, const hw5::Paths& paths) {
        if (choice == "all" || choice == "11") {
            for (std::size_t i = 0; i < tasks.size(); ++i) {
                runTask(i, paths);
            }
        } else {
            const int index = resolveTask(choice);
            if (index < 0) {
                throw std::invalid_argument("Unknown task: " + choice);
            }
            runTask(static_cast<std::size_t>(index), paths);
        }
    }

    void writeParameters(const hw5::Paths& paths) {
        hw5::writeText(paths.output / "parameters.txt", R"(ADIP HW5 experiment parameters
Input: )" + paths.input.string() + "\nOutput: " + paths.output.string() + R"(
OpenCV is used only for cv::dft / cv::idft (Q1-2, Q1-3) and cv::dct / cv::idct (Q2, Q3).
RAW / PNG output, resize, filters, hashing, quantization and metrics are own C++20 code (PNG via stb).
Q1-1 own DFT: separable row-column direct DFT with precomputed twiddle tables, O(MN(M+N)).
Spectrum display: fftshift (DC to center), log(1+|F|) linearly scaled to [0,255].
Q1-3 D(u,v): Euclidean distance to (W/2, H/2). High-pass results: 1%-99% percentile stretch for display.
Q1-3 filtered spectra use the original spectrum's maximum as scale (comparable brightness).
Notch: peaks = 9x9 local maxima of log|F| exceeding their 31x31 mean by 2.0, radius >= 20,
       axes |du|,|dv| <= 2 excluded; Butterworth notch pairs D0 = 6, n = 4.
Q2 resize: own area-averaging (box) resampling. pHash: 32x32 DCT, 8x8 low band,
   mean of 63 AC coefficients (DC excluded), bit = coef > mean. aHash: 8x8, bit = pixel >= mean.
Q3 JPEG: level shift -128, 8x8 cv::dct, round(F/Q), dequantize, cv::idct, +128, clamp/round to uint8.
   Table Q equals QF = 50 (S = 100). QF20: S = 250, QF80: S = 40.
)");
    }
} // namespace

int main(int argc, char* argv[]) {
    try {
        const hw5::Paths paths = hw5::locatePaths(argv[0]);
        writeParameters(paths);
        std::cout << "Input : " << paths.input.string() << "\nOutput: " << paths.output.string()
                  << '\n';
        if (argc > 1) {
            dispatch(argv[1], paths);
            return 0;
        }
        while (true) {
            std::cout << "\n========== ADIP HW5 ==========\n";
            for (std::size_t i = 0; i < tasks.size(); ++i) {
                std::cout << i + 1 << ". [" << tasks[i].id << "] " << tasks[i].title << '\n';
            }
            std::cout << "11. Run all\n0. Exit\nSelect number or task ID: " << std::flush;
            std::string choice;
            if (!std::getline(std::cin, choice) || choice == "0") {
                break;
            }
            try {
                dispatch(choice, paths);
            } catch (const std::exception& error) {
                std::cerr << "[ERROR] " << error.what() << '\n';
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "[ERROR] " << error.what() << '\n';
        return 1;
    }
}
