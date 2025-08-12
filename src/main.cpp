#include <iostream>
#include <raylib.h>
#include <entt/entt.hpp>
#include <string>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "components/BoxCollider2D.h"
#include "components/Transform.h"
#include "nlohmann/json.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define BUILD_ATLAS_MODE 1

using json = nlohmann::json;
namespace fs = std::filesystem;



struct PackedImage {
    std::string name;
    int width, height, channels;
    unsigned char* pixels;
};


void GeraTextureAtlas(const std::string& inputDir, const std::string& outputAtlas, const std::string& outputJson) {
    const int ATLAS_WIDTH = 1024;
    const int ATLAS_HEIGHT = 1024;
    const int MARGIN = 1;

    std::vector<PackedImage> images;

    // 1. Carregar imagens da pasta
    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            int w, h, c;
            unsigned char* data = stbi_load(entry.path().string().c_str(), &w, &h, &c, 4);
            if (data) {
                images.push_back({ entry.path().filename().string(), w, h, 4, data });
            } else {
                std::cerr << "Erro ao carregar imagem: " << entry.path().string() << "\n";
            }
        }
    }

    if (images.empty()) {
        std::cerr << "Nenhuma imagem encontrada em: " << inputDir << "\n";
        return;
    }

    // 2. Preparar retângulos para empacotamento
    std::vector<stbrp_rect> rects(images.size());
    for (size_t i = 0; i < images.size(); ++i) {
        rects[i].id = i;
        rects[i].w = images[i].width + MARGIN * 2;
        rects[i].h = images[i].height + MARGIN * 2;
    }

    // 3. Inicializar o empacotador de retângulos
    const int NUM_NODES = ATLAS_WIDTH;
    std::vector<stbrp_node> nodes(NUM_NODES);
    stbrp_context context;
    stbrp_init_target(&context, ATLAS_WIDTH, ATLAS_HEIGHT, nodes.data(), NUM_NODES);

    if (!stbrp_pack_rects(&context, rects.data(), (int)rects.size())) {
        std::cerr << "Erro: não foi possível empacotar todas as imagens no atlas.\n";
        return;
    }

    // 4. Criar buffer para o atlas
    std::vector<unsigned char> atlas(ATLAS_WIDTH * ATLAS_HEIGHT * 4, 0);

    // 5. Copiar pixels das imagens para o atlas
    json atlasJson;
    for (const auto& rect : rects) {
        const PackedImage& img = images[rect.id];
        int dstX = rect.x + MARGIN;
        int dstY = rect.y + MARGIN;

        for (int y = 0; y < img.height; ++y) {
            for (int x = 0; x < img.width; ++x) {
                int srcIdx = (y * img.width + x) * 4;
                int dstIdx = ((dstY + y) * ATLAS_WIDTH + (dstX + x)) * 4;
                std::memcpy(&atlas[dstIdx], &img.pixels[srcIdx], 4);
            }
        }

        // Salvar dados no JSON
        atlasJson[img.name] = {
            {"x", dstX},
            {"y", dstY},
            {"width", img.width},
            {"height", img.height}
        };
    }

    // 6. Salvar atlas como PNG
    if (!stbi_write_png(outputAtlas.c_str(), ATLAS_WIDTH, ATLAS_HEIGHT, 4, atlas.data(), ATLAS_WIDTH * 4)) {
        std::cerr << "Erro ao salvar o atlas como PNG.\n";
    }

    // 7. Salvar JSON
    std::ofstream jsonFile(outputJson);
    if (jsonFile) {
        jsonFile << atlasJson.dump(4);
        jsonFile.close();
    } else {
        std::cerr << "Erro ao salvar arquivo JSON.\n";
    }

    // 8. Liberar memória das imagens
    for (auto& img : images) {
        stbi_image_free(img.pixels);
    }

    std::cout << "Atlas gerado com sucesso!\n";
}
int main() {

#if BUILD_ATLAS_MODE
    GeraTextureAtlas(RESOURCE_PATH"imagens/", RESOURCE_PATH"atlas.png", RESOURCE_PATH"atlas.json");
#endif

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(800, 600, "raylib + entt - collision demo");
    SetTargetFPS(60);

    entt::registry registry;

    const auto player = registry.create();
    registry.emplace<rpg::Transform>(player);
    registry.emplace<rpg::BoxCollider2D>(player);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        EndDrawing();
    }


    CloseWindow();

    return 0;
}
