#include "hw4/hw4_3_3.hpp"

namespace hw4 {
    void runHw4_3_3(const Paths& paths) {
        const Image image = readRaw(paths.input / "cameraman256_noise.raw", 256, 256);
        saveEdge(paths.output / "hw4_3_3", "noisy", logEdges(image));
    }
} // namespace hw4
