//
// Created by jhone on 12/08/2025.
//

#include "my_scene.h"

#include <iostream>

#include "raylib.h"
#include "../components/components.h"

rpg::MyScene::MyScene(entt::registry *registry): Scene(registry) {}

void rpg::MyScene::init() {
    const auto player = registry->create();

    registry->emplace<ColorRect>(player, Color(30, 200, 25, 255), 100, 100);
    registry->emplace<Transform>(player, Transform(Vector2(100, 100), 0, Vector2(1, 1)));
    registry->emplace<Input>(player, Vector2(0, 0));
    registry->emplace<BoxCollider2D>(player, 100, 100, false, false);

    const auto enemy = registry->create();
    registry->emplace<ColorRect>(enemy, Color(138, 73, 51, 255), 100, 100);
    registry->emplace<Transform>(enemy, Transform(Vector2(600, 400), 0, Vector2(1, 1)) );
    registry->emplace<BoxCollider2D>(enemy, 100, 100, false, false);

}

