// Created by jhone on 12/08/2025.

#include "my_scene.h"
#include <random>
#include "engine/components/components.h"

#include "game/factories/entities_factory.h"
#include "engine/scenes/scene.h"

rpg::MyScene::MyScene(entt::registry *registry): rpg::Scene(registry) {
}

constexpr int ENEMY_QUANTITY = 40;
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
            enemy_config.collider.width = 30.f;
            enemy_config.collider.height = 30.f;
        }else {
            enemy_config.sprite.name = "enemy.png";
            enemy_config.collider.width = 60.f;
            enemy_config.collider.height = 60.f;
        }

        enemy_config.sprite.color.r = distColor(gen);
        enemy_config.sprite.color.g = distColor(gen);
        enemy_config.sprite.color.b = distColor(gen);
        enemy_config.sprite.color.a = 255;

        create_enemy(registry, enemy_config);
    }

    Sprite sprite{"enemy.png", Vector2{10,10}, RAYWHITE };
    Transform transform{ {0.f, 0.f}, 0.f, {1.f, 1.f} };




    for (int i = 0; i < 2000; ++i) {
        float x = distX(gen);
        float y = distY(gen);

        transform.position.x = x;
        transform.position.y = y;
        sprite.color.r = distColor(gen);

        const auto environment = registry->create();
        registry->emplace<Sprite>(environment, sprite);
        registry->emplace<Transform>(environment, transform);



    }

}
