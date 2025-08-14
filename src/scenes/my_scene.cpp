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

    registry->emplace<ColorRect>(player, Color(30, 200, 25, 255), 100.f, 100.f);
    registry->emplace<Transform>(player, Transform(Vector2(100.f, 100.f), 0.f, Vector2(1.f, 1.f)));
    registry->emplace<Input>(player, Vector2(0, 0));
    registry->emplace<BoxCollider2D>(player, 100.f, 100.f, false, false);
    registry->emplace<MovementData>(player, Vector2(0.f,0.f), 300.f, Vector2(0.f,0.f));

    const auto enemy = registry->create();
    registry->emplace<ColorRect>(enemy, Color(138, 73, 51, 255), 100.f, 100.f);
    registry->emplace<Transform>(enemy, Transform(Vector2(600.f, 400.f), 0.f, Vector2(1.f, 1.f)) );
    registry->emplace<BoxCollider2D>(enemy, 100.f, 100.f, false, false);
    registry->emplace<MovementData>(enemy, Vector2(0.f,0.f), 300.f, Vector2(0.f,0.f));

}

