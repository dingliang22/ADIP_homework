#include "hw5/common.hpp"
#include <array>
#include <stdexcept>

namespace hw5 {
namespace {
bool hasInputs(const fs::path& dir) {
    constexpr std::array names{"zurich512x384.raw", "zurich512x384_noise.raw", "lena256.raw",
                               "lena256_gamma_0p3.raw", "lena256_gaussian_sigma30.raw",
                               "lena256_rotation_9deg.raw"};
    for (const char* name : names) if (!fs::is_regular_file(dir / name)) return false;
    return true;
}
fs::path searchUp(fs::path path) {
    if (path.empty()) return {};
    path = fs::absolute(path);
    if (!fs::is_directory(path)) path = path.parent_path();
    while (!path.empty()) {
        for (const fs::path& candidate : {path, path / "hw5_img", path / "hw5" / "hw5_img"})
            if (hasInputs(candidate)) return fs::canonical(candidate);
        const fs::path parent = path.parent_path();
        if (parent == path) break;
        path = parent;
    }
    return {};
}
} // namespace

Paths locatePaths(const fs::path& executable) {
    fs::path input;
    for (const fs::path& start : {executable, fs::current_path(), fs::path(HW5_SOURCE_DIR)}) {
        input = searchUp(start);
        if (!input.empty()) break;
    }
    if (input.empty()) throw std::runtime_error("Cannot find hw5_img with the six RAW inputs");
    const fs::path output = input.parent_path() / "hw5_img_result";
    fs::create_directories(output);
    return {input, fs::weakly_canonical(output)};
}
} // namespace hw5
