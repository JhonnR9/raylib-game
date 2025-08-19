//
// Created by jhone on 12/08/2025.
//

#include "texture_packer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

struct PackedImage {
    std::string name;
    int width, height, channels;
    unsigned char *pixels;
};

namespace fs = std::filesystem;

void TexturePacker::packer(
    const std::string &inputDir,
    const std::string &outputAtlas,
    const std::string &outputJson) const {
    std::vector<PackedImage> images;

    // 1. Load images from the folder
    for (const auto &entry: fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            int w, h, c;
            unsigned char *data = stbi_load(entry.path().string().c_str(), &w, &h, &c, 4);
            if (data) {
                images.push_back({entry.path().filename().string(), w, h, 4, data});
            } else {
                std::cerr << "Failed to load image: " << entry.path().string() << "\n";
            }
        }
    }

    if (images.empty()) {
        std::cerr << "No images found in: " << inputDir << "\n";
        return;
    }

    // 2. Prepare rectangles for packing
    std::vector<stbrp_rect> rects(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        rects[i].id = i;
        rects[i].w = images[i].width + MARGIN * 2;
        rects[i].h = images[i].height + MARGIN * 2;
    }

    // 3. Initialize the rectangle packer
    const int NUM_NODES = ATLAS_WIDTH;
    std::vector<stbrp_node> nodes(NUM_NODES);
    stbrp_context context;
    stbrp_init_target(&context, ATLAS_WIDTH, ATLAS_HEIGHT, nodes.data(), NUM_NODES);

    if (!stbrp_pack_rects(&context, rects.data(), (int) rects.size())) {
        std::cerr << "Error: could not pack all images into the atlas.\n";
        return;
    }

    // 4. Create buffer for the atlas
    std::vector<unsigned char> atlas(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0);

    // 5. Copy image pixels into the atlas
    nlohmann::json atlasJson;
    for (const auto &rect: rects) {
        const PackedImage &img = images[rect.id];
        int dstX = rect.x + MARGIN;
        int dstY = rect.y + MARGIN;

        for (int y = 0; y < img.height; ++y) {
            for (int x = 0; x < img.width; ++x) {
                int srcIdx = (y * img.width + x) * 4;
                int dstIdx = ((dstY + y) * ATLAS_WIDTH + (dstX + x)) * 4;
                std::memcpy(&atlas[dstIdx], &img.pixels[srcIdx], 4);
            }
        }

        // Save data to JSON
        atlasJson[img.name] = {
            {"x", dstX},
            {"y", dstY},
            {"width", img.width},
            {"height", img.height}
        };
    }

    // 6. Save atlas as PNG
    if (!stbi_write_png(outputAtlas.c_str(), ATLAS_WIDTH, ATLAS_HEIGHT, 4, atlas.data(), ATLAS_WIDTH * 4)) {
        std::cerr << "Failed to save atlas as PNG.\n";
    }

    // 7. Save JSON
    std::ofstream jsonFile(outputJson);
    if (jsonFile) {
        jsonFile << atlasJson.dump(4);
        jsonFile.close();
    } else {
        std::cerr << "Failed to save JSON file.\n";
    }

    // 8. Free image memory
    for (auto &img: images) {
        stbi_image_free(img.pixels);
    }

    std::cout << "Atlas successfully generated!\n";
}
