//
// Created by jhone on 12/08/2025.
//

#include "my_scene.h"
#include "factories/entities_factory.h"

rpg::MyScene::MyScene(entt::registry *registry): Scene(registry) {
}

void rpg::MyScene::init() {
    PlayerConfig player_config;
    player_config.color_rect.color = RAYWHITE;
    player_config.transform.position = {100.f, 200.f};
    create_player(registry, player_config);

    EnemyConfig enemy_config;
    enemy_config.transform.position = {200.f, 100.f};
    enemy_config.color_rect.color = {220, 220, 220, 255};
    enemy_config.color_rect.width = 25.f;
    enemy_config.collider.width = 25.f;
    create_enemy(registry, enemy_config);
}

