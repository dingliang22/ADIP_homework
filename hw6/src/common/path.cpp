#include "hw6/common.hpp"
#include <array>
#include <stdexcept>

namespace hw6 {
namespace {
bool hasInputs(const fs::path& dir) {
    constexpr std::array names{"text1480x460_binary.raw", "cell1200x900.raw",
                               "fingerprint360x480.raw", "wake1274x716.raw"};
    for (const char* name : names) if (!fs::is_regular_file(dir / name)) return false;
    return true;
}
fs::path searchUp(fs::path path) {
    if (path.empty()) return {};
    path = fs::absolute(path);
    if (!fs::is_directory(path)) path = path.parent_path();
    while (!path.empty()) {
        for (const fs::path& candidate : {path, path / "hw6_img", path / "hw6" / "hw6_img"})
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
    for (const fs::path& start : {executable, fs::current_path(), fs::path(HW6_SOURCE_DIR)}) {
        input = searchUp(start);
        if (!input.empty()) break;
    }
    if (input.empty()) throw std::runtime_error("Cannot find hw6_img with the four RAW inputs");
    const fs::path output = input.parent_path() / "hw6_img_result";
    fs::create_directories(output);
    return {input, fs::weakly_canonical(output)};
}
} // namespace hw6
