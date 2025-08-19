//
// Created by jhone on 15/08/2025.
//

#include "camera_system.h"

#include "engine/components/components.h"

namespace rpg {

    CameraSystem::CameraSystem(entt::registry *registry): System(registry) {
    }

    void CameraSystem::run(float dt) {
        const auto view = registry->view<Input, rpg::Transform>();
        if (view.begin() == view.end()) return;
        const auto entity = *view.begin();

        const auto &transform = view.get<Transform>(entity);

        if (!is_synced) {
            const auto new_width = static_cast<float>(GetScreenWidth());
            const auto new_height = static_cast<float>(GetScreenHeight());
            camera.target = transform.position;
            camera.offset = (Vector2){new_width / 2.0f, new_height / 2.0f};
            camera.rotation = 0.0f;
            camera.zoom = 2.0f;

            is_synced = true;
        }
        if (IsWindowResized())
        {
            const auto new_width = static_cast<float>(GetScreenWidth());
            const auto new_height = static_cast<float>(GetScreenHeight());

            camera.offset = (Vector2){ new_width / 2.0f, new_height / 2.0f };
        }


        camera.target = {
            std::lerp(camera.target.x, transform.position.x, 0.2f),
            std::lerp(camera.target.y, transform.position.y, 0.2f)
        };
    }
} // rpg
