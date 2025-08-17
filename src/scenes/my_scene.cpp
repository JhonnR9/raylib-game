// Created by jhone on 12/08/2025.

#include "my_scene.h"
#include "factories/entities_factory.h"
#include <random>

rpg::MyScene::MyScene(entt::registry *registry): Scene(registry) {
}

constexpr int ENEMY_QUANTITY = 400;
constexpr float MAP_WIDTH = 2000;
constexpr float MAP_HEIGHT = 2000;
constexpr float ENEMY_SIZE = 45.f;


void rpg::MyScene::init() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_real_distribution<float> distX(0.f, MAP_WIDTH);
    std::uniform_real_distribution<float> distY(0.f, MAP_HEIGHT);
    std::uniform_int_distribution distColor(100, 255);
    std::uniform_int_distribution<> dis(0, 1);

    PlayerConfig player_config;
    player_config.transform.scale = Vector2(1.f,1.f);
    player_config.transform.position = {100.f, 200.f};
    create_player(registry, player_config);

    EnemyConfig enemy_config;
    enemy_config.collider.width = ENEMY_SIZE;
    enemy_config.collider.height = ENEMY_SIZE;
    enemy_config.collider.is_static = false;
    enemy_config.sprite.name = "sprite.png";

    for (int i = 0; i < ENEMY_QUANTITY; ++i) {
        float x = distX(gen);
        float y = distY(gen);
        enemy_config.transform.position = {x, y};
        if (dis(gen) == 1) {
            enemy_config.sprite.name = "sprite.png";
        }else {
            enemy_config.sprite.name = "enemy.png";
        }

        enemy_config.sprite.color.r = distColor(gen);
        enemy_config.sprite.color.g = distColor(gen);
        enemy_config.sprite.color.b = distColor(gen);
        enemy_config.sprite.color.a = 255;

        create_enemy(registry, enemy_config);
    }
}
