// Created by jhone on 12/08/2025.

#include "my_scene.h"
#include "factories/entities_factory.h"
#include <random>

rpg::MyScene::MyScene(entt::registry *registry): Scene(registry) {
}

constexpr int ENEMY_QUANTITY = 20;
constexpr float MAP_WIDTH = 1500;
constexpr float MAP_HEIGHT = 1500;
constexpr float ENEMY_SIZE = 50.f;

void rpg::MyScene::init() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::normal_distribution<float> distX(MAP_WIDTH / 2.f, 300.f);
    std::normal_distribution<float> distY(MAP_HEIGHT / 2.f, 300.f);
    std::uniform_int_distribution distColor(100, 255);

    PlayerConfig player_config;
    player_config.color_rect.color = RAYWHITE;
    player_config.transform.position = {100.f, 200.f};
    create_player(registry, player_config);

    EnemyConfig enemy_config;
    enemy_config.color_rect.width = ENEMY_SIZE;
    enemy_config.color_rect.height = ENEMY_SIZE;
    enemy_config.collider.width = ENEMY_SIZE;
    enemy_config.collider.height = ENEMY_SIZE;
    enemy_config.collider.is_static = false;

    for (int i = 0; i < ENEMY_QUANTITY; ++i) {
        float x = std::clamp(distX(gen), 0.f, MAP_WIDTH);
        float y = std::clamp(distY(gen), 0.f, MAP_HEIGHT);
        enemy_config.transform.position = {x, y};

        enemy_config.color_rect.color.r = distColor(gen);
        enemy_config.color_rect.color.g = distColor(gen);
        enemy_config.color_rect.color.b = distColor(gen);
        enemy_config.color_rect.color.a = 255;

        create_enemy(registry, enemy_config);
    }
}
