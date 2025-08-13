//
// Created by jhone on 13/08/2025.
//

#include "move_system.h"

#include <iostream>


#include "../components/components.h"
#include "raylib.h"
#include "raymath.h"

namespace rpg {
    MoveSystem::MoveSystem(entt::registry *registry): System(registry) {
    }

    void MoveSystem::run(float dt) {
        for (auto view = registry->view<Input, Transform>(); const auto entity: view) {
            auto &&[input, transform] = view.get<Input, Transform>(entity);

            input.move_direction = Vector2Normalize(input.move_direction);

            const Vector2 scaled_direction = {
                input.move_direction.x  * dt *200,
                input.move_direction.y  * dt *200
            };

            transform.position = Vector2Add(transform.position, scaled_direction);

        }
    }

} // rpg
