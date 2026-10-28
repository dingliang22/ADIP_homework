#include "hw4/common.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <fstream>
#include <stdexcept>

namespace hw4 {
void saveImage(const fs::path& stem, const Image& image) {
    const Image output = toByteImage(image);
    if (!stem.parent_path().empty()) fs::create_directories(stem.parent_path());
    std::vector<unsigned char> bytes(output.pixels.begin(), output.pixels.end());
    const fs::path raw = stem.string() + ".raw";
    std::ofstream stream(raw, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    stream.close();
    if (!stream) throw std::runtime_error("Cannot write: " + raw.string());
    const fs::path png = stem.string() + ".png";
    if (!stbi_write_png(png.string().c_str(), output.width, output.height, 1,
                        bytes.data(), output.width))
        throw std::runtime_error("Cannot write: " + png.string());
}
} // namespace hw4
