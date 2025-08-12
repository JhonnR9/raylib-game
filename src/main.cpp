#include <iostream>
#include <raylib.h>
#include <entt/entt.hpp>

#include "rlgl.h"

struct Position {
    Vector2 position;
};

struct Velocity {
    Vector2 velocity;
};

struct RenderColor {
    Color color;
};


void UpdatePosition(entt::registry& registry, float deltaTime);


int main() {
    InitWindow(800, 600, "raylib");
    SetTargetFPS(60);

    entt::registry registry;

    auto entity1 = registry.create();
    registry.emplace<Position>(entity1,  Vector2{10, 10});
    registry.emplace<Velocity>(entity1, Vector2{10, 0});
    registry.emplace<RenderColor>(entity1 ,YELLOW);

    auto entity2 = registry.create();
    registry.emplace<Position>(entity2, Vector2{10, 10});
    registry.emplace<Velocity>(entity2, Vector2{0, 10});
    registry.emplace<RenderColor>(entity2 ,RED);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        UpdatePosition(registry, GetFrameTime());

        auto view = registry.view<Position, RenderColor>();
        for (auto entity : view) {
            Vector2 position = view.get<Position>(entity).position;
            auto color = view.get<RenderColor>(entity).color;

            DrawRectangle(position.x, position.y, 50, 50, color);

        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void UpdatePosition(entt::registry &registry, float deltaTime) {
    auto view = registry.view<Position, Velocity>();
    for (auto entity : view) {
        auto& position = view.get<Position>(entity);
        auto& velocity = view.get<Velocity>(entity);

        position.position.x += velocity.velocity.x * deltaTime;
        position.position.y += velocity.velocity.y * deltaTime;

    }
}