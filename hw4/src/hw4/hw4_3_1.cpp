#include "hw4/hw4_3_1.hpp"

namespace hw4 {
    void runHw4_3_1(const Paths& paths) {
        for (const std::string prefix : {"clean", "noisy"}) {
            const Image image = readRaw(
                paths.input / (prefix == "clean" ? "cameraman256.raw" : "cameraman256_noise.raw"),
                256, 256);
            for (bool eight : {false, true}) {
                saveEdge(paths.output / "hw4_3_1", prefix, laplacian(image, eight));
            }
        }
    }
} // namespace hw4
