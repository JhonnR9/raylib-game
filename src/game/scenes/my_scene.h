//
// Created by jhone on 12/08/2025.
//

#ifndef MY_SCENE_H
#define MY_SCENE_H
#include "entt/entt.hpp"
#include "engine/scenes/scene.h"

namespace rpg {
    class RenderSystem;

    class MyScene final : public Scene {
    public:
        explicit MyScene(entt::registry* registry);
        void init() override;

    };
} //rpg


#endif //MY_SCENE_H
