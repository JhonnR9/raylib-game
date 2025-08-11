#include <raylib.h>
#include <entt/entt.hpp>

struct Position { float x, y; };
struct Velocity { float vx, vy; };
struct Renderable { Texture2D texture; };

void RenderSystem(entt::registry& registry) {
    auto view = registry.view<Position, Renderable>();
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& rend = view.get<Renderable>(entity);
        DrawTexture(rend.texture, (int)pos.x, (int)pos.y, WHITE);
    }
}

int main() {
    InitWindow(800, 600, "Viewport");
    SetTargetFPS(60);

    entt::registry registry;
    auto entity = registry.create();

    registry.emplace<Position>(entity, 100.0f, 100.0f);
    registry.emplace<Velocity>(entity, 10.0f, 10.5f);

    // Carregando textura
    const Texture2D texture = LoadTexture(RESOURCE_PATH"/texture.jpg");
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);

    // Adiciona o componente Renderable com a textura
    registry.emplace<Renderable>(entity, texture);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Atualiza posição baseado na velocidade
        auto& pos = registry.get<Position>(entity);
        auto& vel = registry.get<Velocity>(entity);
        pos.x += vel.vx * deltaTime;
        pos.y += vel.vy * deltaTime;

        BeginDrawing();
        ClearBackground(BLACK);

        // Chama o sistema de renderização
        RenderSystem(registry);

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, BLUE);

        EndDrawing();
    }

    UnloadTexture(texture);
    CloseWindow();
    return 0;
}
