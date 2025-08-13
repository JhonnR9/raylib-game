//
// Created by jhone on 12/08/2025.
//

#include "app.h"
#include <iostream>
#include <raylib.h>
#include "scenes/my_scene.h"
#include "systems/collider_system.h"
#include "systems/move_system.h"
#include "systems/player_input_system.h"
#include "systems/render_system.h"
#if BUILD_ATLAS_MODE
#include "utils/texture_packer.h"
#endif


namespace rpg {
    APP::APP() {
#if BUILD_ATLAS_MODE
        const TexturePacker *texture_tool = new TexturePacker();
        texture_tool->packer(RESOURCE_PATH"/imagens/", RESOURCE_PATH"/atlas.png", RESOURCE_PATH"/atlas.json");
        delete texture_tool;
#endif

        SetTraceLogLevel(LOG_ERROR);
        InitWindow(800, 600, "raylib + entt - collision demo");
        SetTargetFPS(60);

        registry = std::make_unique<entt::registry>();
        scene = std::make_unique<MyScene>(registry.get());

        auto render_system = std::make_unique<RenderSystem>(registry.get());
        systems.push_back(std::move(render_system));

        auto player_input_system = std::make_unique<PlayerInputSystem>(registry.get());
        systems.push_back(std::move(player_input_system));

        auto move_system = std::make_unique<MoveSystem>(registry.get());
        systems.push_back(std::move(move_system));

        auto collision_system = std::make_unique<ColliderSystem>(registry.get());
        systems.push_back(std::move(collision_system));
    }

    APP::~APP() {
        CloseWindow();
    }

    void APP::run() const {
        scene->init();

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(BLACK);

            for (const auto &system : systems) {
                system->run(GetFrameTime());
            }
            DrawFPS(10, 10);

            EndDrawing();
        }
    }
} // rpg
