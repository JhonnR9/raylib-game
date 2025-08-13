//
// Created by jhone on 12/08/2025.
//

#include "render_system.h"
#include <raylib.h>
#include <algorithm>
#include <iostream>

#include "raymath.h"
#include "../components/components.h"

namespace rpg {
    RenderSystem::RenderSystem(entt::registry *registry): System(registry) {
    }

    void RenderSystem::run(float dt) {
        auto view = registry->view<ColorRect, Transform>();

        for (const auto entity: view) {
            auto &color_rect = view.get<ColorRect>(entity);
            const auto &transform = view.get<rpg::Transform>(entity);

            const Rectangle rec(
                transform.position.x,
                transform.position.y,
                color_rect.width, color_rect.height
            );

            DrawRectanglePro(
                rec,
                transform.position/2,
                transform.rotation,
                color_rect.color);
        }
    }
} // rpg
