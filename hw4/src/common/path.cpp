#include "hw4/common.hpp"
#include <array>
#include <stdexcept>

namespace hw4 {
namespace {
bool hasInputs(const fs::path& dir) {
    constexpr std::array names{"town500.raw", "town500_noisy.raw", "lena256.raw", "lena512.raw",
                               "cameraman256.raw", "cameraman256_noise.raw"};
    for (const char* name : names) if (!fs::is_regular_file(dir / name)) return false;
    return true;
}
fs::path searchUp(fs::path path) {
    if (path.empty()) return {};
    path = fs::absolute(path);
    if (!fs::is_directory(path)) path = path.parent_path();
    while (!path.empty()) {
        for (const fs::path& candidate : {path, path / "hw4_img", path / "hw4" / "hw4_img"})
            if (hasInputs(candidate)) return fs::canonical(candidate);
        const fs::path parent = path.parent_path();
        if (parent == path) break;
        path = parent;
    }
    return {};
}
} // namespace
Paths locatePaths(const fs::path& executable, const fs::path& inputOverride,
                  const fs::path& outputOverride) {
    fs::path input;
    // Explicit paths always take precedence and are never silently ignored.
    if (!inputOverride.empty()) {
        if (!hasInputs(inputOverride))
            throw std::runtime_error("Input directory must contain all six RAW files: " + inputOverride.string());
        input = fs::canonical(inputOverride);
    } else {
        for (const fs::path& start : {executable, fs::current_path(), fs::path(HW4_SOURCE_DIR)}) {
            input = searchUp(start);
            if (!input.empty()) break;
        }
    }
    if (input.empty()) throw std::runtime_error("Cannot find hw4_img; use --img-dir PATH");
    const fs::path output = outputOverride.empty() ? input.parent_path() / "hw4_img_result"
                                                 : fs::absolute(outputOverride);
    if (fs::weakly_canonical(output) == input)
        throw std::runtime_error("Output directory must differ from the input directory");
    fs::create_directories(output);
    return {input, fs::weakly_canonical(output)};
}
} // namespace hw4
