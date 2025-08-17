// app.h
// Author: Jhone
// Created: 12/08/2024
// Purpose: Main application class for a 2D game using raylib and entt,
// managing the game loop, scene, and ECS systems.

#include "app.h"
#include <iostream>
#include <raylib.h>
#include "scenes/my_scene.h"
#include "systems/collision_detection_system.h"
#include "systems/move_system.h"
#include "systems/overlap_correction_system.h"
#include "systems/player_input_system.h"
#include "systems/shape_render_system.h"

#include "systems/camera_system.h"
#include "systems/sprite_renderer_system.h"

#if BUILD_ATLAS_MODE
#include "utils/texture_packer.h"
#endif


namespace rpg {
    APP::APP() {
#if BUILD_ATLAS_MODE
        const TexturePacker *texture_tool = new TexturePacker();
        texture_tool->packer(RESOURCE_PATH"/images/", RESOURCE_PATH"/atlas.png", RESOURCE_PATH"/atlas.json");
        delete texture_tool;
#endif

        SetTraceLogLevel(LOG_ERROR);
        InitWindow(800, 600, "raylib + entt - collision demo");
        //SetTargetFPS(60);

        registry = std::make_unique<entt::registry>();
        scene = std::make_unique<MyScene>(registry.get());

        auto player_input_system = std::make_unique<PlayerInputSystem>(registry.get());
        systems.push_back(std::move(player_input_system));

        auto move_system = std::make_unique<MoveSystem>(registry.get());
        systems.push_back(std::move(move_system));

        auto collision_detection_system = std::make_unique<CollisionDetectionSystem>(registry.get());
        systems.push_back(std::move(collision_detection_system));

        auto overlap_correction_system = std::make_unique<OverlapCorrectionSystem>(registry.get());
        systems.push_back(std::move(overlap_correction_system));

        auto camera_system = std::make_unique<CameraSystem>(registry.get());
        camera = camera_system->get_camera();
        systems.push_back(std::move(camera_system));

         auto shape_render_system = std::make_unique<RenderSystem>(registry.get());
         systems.push_back(std::move(shape_render_system));

        auto sprite_render_system = std::make_unique<SpriteRendererSystem>(registry.get());
        systems.push_back(std::move(sprite_render_system));


    }

    APP::~APP() {
        CloseWindow();
    }

    void APP::run() const {
        scene->init();

        while (!WindowShouldClose()) {


            BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(*camera);
            for (const auto &system : systems) {
                system->run(GetFrameTime());
            }
            EndMode2D();
            DrawFPS(10, 10);

            EndDrawing();
        }
    }
} // rpg
