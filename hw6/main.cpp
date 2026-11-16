#include "hw6/common.hpp"
#include "hw6/hw6_1_1.hpp"
#include "hw6/hw6_1_2.hpp"
#include "hw6/hw6_2_1.hpp"
#include "hw6/hw6_2_2.hpp"
#include "hw6/hw6_3.hpp"

#include <array>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {
    struct Task {
        const char* id;
        const char* title;
        void (*run)(const hw6::Paths&);
    };

    const std::array<Task, 5> tasks{
        {{"1-1", "Restore text1480x460 (binary morphology)", hw6::runHw6_1_1},
         {"1-2", "Restore cell1200x900 (grayscale morphology)", hw6::runHw6_1_2},
         {"2-1", "Fingerprint thinning (Zhang-Suen)", hw6::runHw6_2_1},
         {"2-2", "Ridge endings / bifurcations (crossing number)", hw6::runHw6_2_2},
         {"3", "Top-hat / Bottom-hat enhancement", hw6::runHw6_3}}};

    void runTask(std::size_t index, const hw6::Paths& paths) {
        std::cout << "\n[" << tasks[index].id << "] " << tasks[index].title << '\n';
        tasks[index].run(paths);
        std::cout << "Completed " << tasks[index].id << '\n';
    }

    int resolveTask(const std::string& choice) {
        // Task IDs first ("3" is question 3), then menu numbers.
        for (std::size_t i = 0; i < tasks.size(); ++i) {
            if (choice == tasks[i].id) {
                return static_cast<int>(i);
            }
        }
        for (std::size_t i = 0; i < tasks.size(); ++i) {
            if (choice == std::to_string(i + 1)) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void dispatch(const std::string& choice, const hw6::Paths& paths) {
        if (choice == "all" || choice == "6") {
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

int main(int argc, char* argv[]) {
    try {
        const hw6::Paths paths = hw6::locatePaths(argv[0]);
        std::cout << "Input : " << paths.input.string() << "\nOutput: " << paths.output.string()
                  << '\n';
        if (argc > 1) {
            dispatch(argv[1], paths);
            return 0;
        }
        while (true) {
            std::cout << "\n========== ADIP HW6 ==========\n";
            for (std::size_t i = 0; i < tasks.size(); ++i) {
                std::cout << i + 1 << ". [" << tasks[i].id << "] " << tasks[i].title << '\n';
            }
            std::cout << "6. Run all\n0. Exit\nSelect number or task ID: " << std::flush;
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
