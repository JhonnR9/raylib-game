//
// Created by jhone on 12/08/2025.
//

#include "my_scene.h"

#include <iostream>

#include "raylib.h"
#include "../components/color_rect.h"
#include "../components/transform.h"


rpg::MyScene::MyScene(entt::registry *registry): Scene(registry) {
}

void rpg::MyScene::init() {
    const auto player = registry->create();
    registry->emplace<ColorRect>(player, Color(255, 255, 255, 255), 100, 100);
    constexpr auto position = Vector3(100, 100, 0);
    constexpr auto rotation = Quaternion(0, 0, 0, 0);
    constexpr auto scale = Vector3(1, 1, 1);
    registry->emplace<rpg::Transform>(player, Transform(position, rotation, scale));
}

void rpg::MyScene::loop(float dt) {
}
