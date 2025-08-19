//
// Created by jhone on 15/08/2025.
//

#ifndef CAMERA_SYSTEM_H
#define CAMERA_SYSTEM_H
#include "system.h"
#include "raylib.h"
#include "entt/entt.hpp"

namespace rpg {
    class CameraSystem final : public System {
        Camera2D camera{0};
        bool is_synced = false;

    public:
        explicit CameraSystem(entt::registry *registry);

        void run(float dt) override;

        Camera2D *get_camera() { return &camera; }
    };
} // rpg

#endif //CAMERA_SYSTEM_H
