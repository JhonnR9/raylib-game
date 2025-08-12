//
// Created by jhone on 12/08/2025.
//

#include "render_system.h"
#include <raylib.h>
#include <algorithm>
#include <iostream>

#include "../components/color_rect.h"
#include "../components/transform.h"

namespace rpg {

    RenderSystem::RenderSystem(entt::registry* registry): System(registry) {
    }

    void RenderSystem::run(float dt) {

        auto view = registry->view<ColorRect, Transform>();

        for (auto entity : view) {

            auto &color_rect = view.get<ColorRect>(entity);
            auto &transform = view.get<rpg::Transform>(entity);
            DrawRectangle(transform.position.x, transform.position.y, color_rect.width, color_rect.height, color_rect.color);
        }
    }
} // rpg