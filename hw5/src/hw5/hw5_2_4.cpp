#include "hw5/hw5_2_4.hpp"
#include <iostream>

namespace hw5 {
// Q2-4: side-by-side pHash / aHash Hamming distances for discussion.
void runHw5_2_4(const Paths& paths) {
    const std::string csv = hashReport(loadLenaSet(paths), paths.output / "hw5_2_4");
    writeText(paths.output / "hw5_2_4" / "phash_vs_ahash.csv", csv);
    std::cout << csv;
}
} // namespace hw5
