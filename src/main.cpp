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

struct Sprite {
    Texture &texture;
};

void MoveSystem(entt::registry &registry, float deltaTime);

void RenderSystem(entt::registry &registry);

int main() {
    InitWindow(800, 600, "raylib");
    SetTargetFPS(60);
    Texture texture = LoadTexture(RESOURCE_PATH"/sprite.png");


    entt::registry registry;

    auto entity1 = registry.create();
    registry.emplace<Position>(entity1, Vector2{10, 10});
    registry.emplace<Velocity>(entity1, Vector2{10, 0});
    registry.emplace<Sprite>(entity1, texture);

    auto entity2 = registry.create();
    registry.emplace<Position>(entity2, Vector2{10, 10});
    registry.emplace<Velocity>(entity2, Vector2{0, 10});
    registry.emplace<Sprite>(entity2, texture);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        MoveSystem(registry, GetFrameTime());
        RenderSystem(registry);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void MoveSystem(entt::registry &registry, float deltaTime) {
    auto view = registry.view<Position, Velocity>();
    for (auto entity: view) {
        auto &position = view.get<Position>(entity);
        auto &velocity = view.get<Velocity>(entity);

        position.position.x += velocity.velocity.x * deltaTime;
        position.position.y += velocity.velocity.y * deltaTime;
    }
}

void RenderSystem(entt::registry &registry) {
    auto view = registry.view<Position, Sprite>();
    for (auto entity: view) {
        auto &sprite = view.get<Sprite>(entity);
        auto &position = view.get<Position>(entity);

        DrawTexture(sprite.texture, position.position.x, position.position.y, WHITE);
    }
}
