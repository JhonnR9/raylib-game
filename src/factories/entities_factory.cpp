//
// Created by jhone on 15/08/2025.
//

#include "entities_factory.h"

#include <iostream>

void rpg::create_player(entt::registry *registry, const PlayerConfig& config) {
    const auto player = registry->create();

    registry->emplace<Sprite>(player,config.sprite);
    registry->emplace<Transform>(player, config.transform);
    registry->emplace<Input>(player, config.input);
    registry->emplace<BoxCollider2D>(player, config.collider);
    registry->emplace<MovementData>(player,config.movement_data);

}

void rpg::create_enemy(entt::registry *registry, const EnemyConfig& config) {
    const auto enemy = registry->create();
    registry->emplace<Sprite>(enemy, config.sprite);
    registry->emplace<Transform>(enemy, config.transform);
    registry->emplace<BoxCollider2D>(enemy, config.collider);
    registry->emplace<MovementData>(enemy, config.movement_data);

}
