
#include "hw3/common.hpp"

// ============================================================
// HW3 Part 1: Histogram Processing
// ============================================================

#include "hw3/hw3_1_1.hpp"
#include "hw3/hw3_1_2.hpp"
#include "hw3/hw3_1_3.hpp"

// ============================================================
// HW3 Part 2: Gray-Level Transformation
// ============================================================

#include "hw3/hw3_2_1.hpp"
#include "hw3/hw3_2_2.hpp"

// ============================================================
// HW3 Part 3: Quantization and Bit-plane Analysis
// ============================================================

#include "hw3/hw3_3_1.hpp"
#include "hw3/hw3_3_2.hpp"

// ============================================================
// HW3 Part 4: Watermarking
// ============================================================

#include "hw3/hw3_4_1.hpp"
#include "hw3/hw3_4_2.hpp"
#include "hw3/hw3_4_3.hpp"

#include <exception>
#include <iostream>
#include <string>

// ============================================================
// Main
// ============================================================

int main(int argc, char* argv[]) {
    const std::string studentId = "115318056";

    while (true) {
        // ----------------------------------------------------
        // Display Main Menu
        // ----------------------------------------------------

        std::cout << "\n";
        std::cout << "==================================================\n";
        std::cout << "                    ADIP HW3                      \n";
        std::cout << "==================================================\n";
        std::cout << "\n";
        std::cout << "  1. HW3 1-1 Global Histogram Equalization\n";
        std::cout << "  2. HW3 1-2 Local Histogram Equalization\n";
        std::cout << "  3. HW3 1-3 Histogram Matching\n";
        std::cout << "  4. HW3 2-1 Negative Transformation\n";
        std::cout << "  5. HW3 2-2 Power-law (Gamma) Transformation\n";
        std::cout << "  6. HW3 3-1 Top-k MSB Selection (PSNR >= 35 dB)\n";
        std::cout << "  7. HW3 3-2 Bit-plane Structure Selection\n";
        std::cout << "  8. HW3 4-1 LSB Watermark\n";
        std::cout << "  9. HW3 4-2 Block Median QIM\n";
        std::cout << "  10.HW3 4-3 6-bit Robustness Test\n";
        std::cout << "  0. Exit\n";
        std::cout << "==================================================\n";
        std::cout << "Select: ";

        // ----------------------------------------------------
        // Read User Selection
        // ----------------------------------------------------

        std::string choice;

        if (!std::getline(std::cin, choice)) {
            break;
        }

        // ----------------------------------------------------
        // Exit
        // ----------------------------------------------------

        if (choice == "0") {
            std::cout << "Program terminated.\n";
            break;
        }

        // ----------------------------------------------------
        // Validate Selection
        // ----------------------------------------------------

        const bool validChoice = choice == "1" || choice == "2" || choice == "3" || choice == "4" ||
                                 choice == "5" || choice == "6" || choice == "7" || choice == "8" ||
                                 choice == "9" || choice == "10";

        if (!validChoice) {
            std::cout << "[WARNING] Invalid selection.\n";
            continue;
        }

        try {
            // ------------------------------------------------
            // Locate Input Image Folder
            // ------------------------------------------------

            const hw3::fs::path inputFolder = hw3::locateImageFolder(argc, argv);

            // ------------------------------------------------
            // Set Output Root Folder
            // ------------------------------------------------

            const hw3::fs::path outputRoot = inputFolder.parent_path() / "hw3_img_result";

            std::cout << "\n";
            std::cout << "Input folder : " << inputFolder << '\n';

            std::cout << "Output root  : " << outputRoot << "\n\n";

            // ------------------------------------------------
            // Part 1: Histogram Processing
            // ------------------------------------------------

            if (choice == "1") {
                hw3::runHw3_1_1(inputFolder, outputRoot);

            } else if (choice == "2") {
                hw3::runHw3_1_2(inputFolder, outputRoot);

            } else if (choice == "3") {
                hw3::runHw3_1_3(inputFolder, outputRoot);

            }

            // ------------------------------------------------
            // Part 2: Gray-Level Transformation
            // ------------------------------------------------

            else if (choice == "4") {
                hw3::runHw3_2_1(inputFolder, outputRoot);

            } else if (choice == "5") {
                hw3::runHw3_2_2(inputFolder, outputRoot);

            }

            // ------------------------------------------------
            // Part 3: Quantization and Bit-plane Analysis
            // ------------------------------------------------

            else if (choice == "6") {
                hw3::runHw3_3_1(inputFolder, outputRoot);

            } else if (choice == "7") {
                hw3::runHw3_3_2(inputFolder, outputRoot);

            }

            // ------------------------------------------------
            // Part 4: Watermarking
            // ------------------------------------------------

            else if (choice == "8") {
                hw3::runHw3_4_1(inputFolder, outputRoot, studentId);

            } else if (choice == "9") {
                hw3::runHw3_4_2(inputFolder, outputRoot, studentId);

            } else if (choice == "10") {
                hw3::runHw3_4_3(inputFolder, outputRoot, studentId);
            }

            // ------------------------------------------------
            // Task Completed
            // ------------------------------------------------

            std::cout << "\n";
            std::cout << "========================================\n";
            std::cout << "     Task completed successfully!       \n";
            std::cout << "========================================\n";

        } catch (const std::exception& e) {
            std::cerr << "\n";
            std::cerr << "[ERROR] " << e.what() << '\n';

            std::cerr << "Task failed. Returning to menu.\n";
        }
    }

    return 0;
}
