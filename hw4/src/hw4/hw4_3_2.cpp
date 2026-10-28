#include "hw4/hw4_3_2.hpp"

namespace hw4 {
    void runHw4_3_2(const Paths& paths) {
        for (const std::string prefix : {"clean", "noisy"}) {
            const Image image = readRaw(
                paths.input / (prefix == "clean" ? "cameraman256.raw" : "cameraman256_noise.raw"),
                256, 256);
            for (int direction : {0, 90, -1}) {
                saveEdge(paths.output / "hw4_3_2", prefix, sobel(image, direction));
            }
        }
    }
} // namespace hw4
