#include "player_input_system.h"

#include "../components/components.h"

rpg::PlayerInputSystem::PlayerInputSystem(entt::registry *registry): System(registry) {
}

void rpg::PlayerInputSystem::run(float dt) {
    auto view = registry->view<Input>();
    for (const auto entity: view) {
        auto &[move_direction] = view.get<Input>(entity);

        if (IsKeyDown(KEY_RIGHT)) {
            move_direction.x = 1.0f;
        } else if (IsKeyDown(KEY_LEFT)) {
            move_direction.x = -1.0f;
        } else {
            move_direction.x = 0.0f;
        }
        if (IsKeyDown(KEY_UP)) {
            move_direction.y = -1.0f;
        } else if (IsKeyDown(KEY_DOWN)) {
            move_direction.y = 1.0f;
        } else {
            move_direction.y = 0.0f;
        }
    }
}
