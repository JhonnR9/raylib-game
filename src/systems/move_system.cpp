//
// Created by jhone on 13/08/2025.
//

#include "move_system.h"
#include "components/components.h"
#include "raylib.h"
#include "raymath.h"

namespace rpg {
    MoveSystem::MoveSystem(entt::registry *registry): System(registry) {
    }

    void MoveSystem::run(float dt) {
        for (auto view = registry->view<Input, Transform, MovementData>(); const auto entity: view) {
            auto &&[input, transform, movement_data] = view.get<Input, Transform, MovementData>(entity);

            movement_data.previous_position = transform.position;
            input.move_direction = Vector2Normalize(input.move_direction);

            movement_data.velocity = {
                input.move_direction.x  * movement_data.speed,
                input.move_direction.y  * movement_data.speed
            };

            transform.position = Vector2Add(transform.position, Vector2Scale(movement_data.velocity, dt));
        }
    }
} // rpg
