#include "hw4/common.hpp"
#include "hw4/hw4_1_1.hpp"
#include "hw4/hw4_1_2.hpp"
#include "hw4/hw4_1_3.hpp"
#include "hw4/hw4_1_4.hpp"
#include "hw4/hw4_2_1.hpp"
#include "hw4/hw4_2_2.hpp"
#include "hw4/hw4_3_1.hpp"
#include "hw4/hw4_3_2.hpp"
#include "hw4/hw4_3_3.hpp"
#include "hw4/hw4_3_4.hpp"

#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {
    struct Task {
        const char* id;
        const char* title;
        void (*run)(const hw4::Paths&);
    };

    const std::array<Task, 10> tasks{
        {{"1-1", "Mean / Gaussian denoising", hw4::runHw4_1_1},
         {"1-2", "Median denoising", hw4::runHw4_1_2},
         {"1-3", "Cascade filtering", hw4::runHw4_1_3},
         {"1-4", "Denoising PSNR / cascade order", hw4::runHw4_1_4},
         {"2-1", "IBP: 1 / 3 / 5 / 10 iterations", hw4::runHw4_2_1},
         {"2-2", "Nearest / Bilinear / IBP comparison", hw4::runHw4_2_2},
         {"3-1", "4 / 8-neighbor Laplacian", hw4::runHw4_3_1},
         {"3-2", "Sobel 0 / 90 / combined", hw4::runHw4_3_2},
         {"3-3", "Laplacian of Gaussian", hw4::runHw4_3_3},
         {"3-4", "Binary edges / Pratt FOM / threshold sweep", hw4::runHw4_3_4}}};

    void runTask(std::size_t index, const hw4::Paths& paths) {
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

    void dispatch(const std::string& choice, const hw4::Paths& paths) {
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
} // namespace

int main(int _, char* argv[]) {
    try {
        hw4::fs::path input, output;
        std::string selection;
        const hw4::Paths paths = hw4::locatePaths(argv[0], input, output);
        hw4::writeParameters(paths);
        std::cout << "Input : " << paths.input.string() << "\nOutput: " << paths.output.string()
                  << '\n';
        if (!selection.empty()) {
            dispatch(selection, paths);
            return 0;
        }
        while (true) {
            std::cout << "\n========== ADIP HW4 ==========\n";
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
