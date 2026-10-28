#include "hw4/hw4_1_3.hpp"

namespace hw4 {
    void runHw4_1_3(const Paths& paths) {
        const Image noisy = readRaw(paths.input / "town500_noisy.raw", 500, 500);
        saveImages(paths.output / "hw4_1_3", denoiseCascades(noisy));
    }
} // namespace hw4
