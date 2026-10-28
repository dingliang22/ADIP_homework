#include "hw4/hw4_1_1.hpp"
namespace hw4 {
void runHw4_1_1(const Paths& paths) {
    const Image noisy = readRaw(paths.input / "town500_noisy.raw", 500, 500);
    saveImages(paths.output / "hw4_1_1", denoiseSingles(noisy));
}
} // namespace hw4
