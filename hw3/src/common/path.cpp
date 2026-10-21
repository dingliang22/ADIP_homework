
#include "hw3/common.hpp"

#include <stdexcept>

namespace hw3 {

    // ============================================================
    // 1. Find hw3_img folder
    //
    // Supported structures:
    //
    // Structure A:
    //   hw3/
    //   └── hw3_img/
    //
    // Structure B:
    //   ADIP_homework/
    //   └── hw3/
    //       └── hw3_img/
    //
    // Search upward through parent directories.
    // ============================================================

    fs::path findHw3Img(const fs::path& inputPath) {
        if (inputPath.empty()) {
            return {};
        }

        // Convert relative path to absolute path.
        fs::path path = fs::absolute(inputPath);

        // If the input is a file, start from its directory.
        if (!fs::is_directory(path)) {
            path = path.parent_path();
        }

        while (!path.empty()) {
            // ----------------------------------------------------
            // Case 1:
            // The current directory itself is hw3_img.
            // ----------------------------------------------------

            if (path.filename() == "hw3_img" && fs::is_directory(path)) {
                return path;
            }

            // ----------------------------------------------------
            // Case 2:
            // Current directory directly contains hw3_img.
            //
            // Example:
            // ADIP_homework/hw3/hw3_img
            // ----------------------------------------------------

            const fs::path directCandidate = path / "hw3_img";

            if (fs::is_directory(directCandidate)) {
                return directCandidate;
            }

            // ----------------------------------------------------
            // Case 3:
            // Current directory contains hw3/hw3_img.
            //
            // Example:
            // ADIP_homework/hw3/hw3_img
            // ----------------------------------------------------

            const fs::path projectCandidate = path / "hw3" / "hw3_img";

            if (fs::is_directory(projectCandidate)) {
                return projectCandidate;
            }

            // ----------------------------------------------------
            // Move to parent directory.
            // ----------------------------------------------------

            const fs::path parent = path.parent_path();

            // Stop at filesystem root.
            if (parent == path) {
                break;
            }

            path = parent;
        }

        return {};
    }

    // ============================================================
    // 2. Locate Image Folder
    //
    // Search priority:
    //
    // 1. argv[0]       : Executable path
    // 2. argv[1]       : User-provided path (if any)
    // 3. current_path  : Current working directory
    // 4. __FILE__      : Source file path
    //
    // Each search supports both:
    //   hw3_img/
    //   hw3/hw3_img/
    // ============================================================

    fs::path locateImageFolder(int argc, char* argv[]) {
        fs::path folder;

        // --------------------------------------------------------
        // Priority 1: Executable path (argv[0])
        // --------------------------------------------------------

        if (argc > 0 && argv != nullptr && argv[0] != nullptr) {
            folder = findHw3Img(argv[0]);

            if (!folder.empty()) {
                return folder;
            }
        }

        // --------------------------------------------------------
        // Priority 2: Command-line argument (argv[1])
        // --------------------------------------------------------

        if (argc > 1 && argv != nullptr && argv[1] != nullptr) {
            folder = findHw3Img(argv[1]);

            if (!folder.empty()) {
                return folder;
            }
        }

        // --------------------------------------------------------
        // Priority 3: Current working directory
        // --------------------------------------------------------

        folder = findHw3Img(fs::current_path());

        if (!folder.empty()) {
            return folder;
        }

        // --------------------------------------------------------
        // Priority 4: Source file path
        // --------------------------------------------------------

        folder = findHw3Img(__FILE__);

        if (!folder.empty()) {
            return folder;
        }

        // --------------------------------------------------------
        // No valid image folder found.
        // --------------------------------------------------------

        throw std::runtime_error("Cannot find hw3_img. "
                                 "Expected hw3_img/ or hw3/hw3_img/ "
                                 "under the project directory.");
    }

} // namespace hw3
