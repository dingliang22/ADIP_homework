#include "hw6/common.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <fstream>
#include <stdexcept>

namespace hw6 {
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

void saveRgbPng(const fs::path& file, const Image& r, const Image& g, const Image& b) {
    const Image rr = toByteImage(r), gg = toByteImage(g), bb = toByteImage(b);
    if (!file.parent_path().empty()) fs::create_directories(file.parent_path());
    std::vector<unsigned char> bytes(rr.pixels.size() * 3);
    for (std::size_t i = 0; i < rr.pixels.size(); ++i) {
        bytes[3 * i] = static_cast<unsigned char>(rr.pixels[i]);
        bytes[3 * i + 1] = static_cast<unsigned char>(gg.pixels[i]);
        bytes[3 * i + 2] = static_cast<unsigned char>(bb.pixels[i]);
    }
    if (!stbi_write_png(file.string().c_str(), rr.width, rr.height, 3, bytes.data(), rr.width * 3))
        throw std::runtime_error("Cannot write: " + file.string());
}
} // namespace hw6
