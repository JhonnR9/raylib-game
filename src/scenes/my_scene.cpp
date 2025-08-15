//
// Created by jhone on 12/08/2025.
//

#include "my_scene.h"
#include "factories/entities_factory.h"


rpg::MyScene::MyScene(entt::registry *registry): Scene(registry) {
}
// Constantes configuráveis
constexpr int ENEMY_QUANTITY = 1000;   // Quantidade de inimigos
constexpr float MAP_WIDTH = 10000.f;    // Largura do mapa
constexpr float MAP_HEIGHT = 8000.f;    // Altura do mapa
constexpr float ENEMY_SIZE = 50.f;     // Tamanho dos inimigos
void rpg::MyScene::init() {
    // Inicializa srand para gerar números aleatórios diferentes a cada execução
    srand(static_cast<unsigned int>(time(nullptr)));

    PlayerConfig player_config;
    player_config.color_rect.color = RAYWHITE;
    player_config.transform.position = {100.f, 200.f};
    create_player(registry, player_config);

    EnemyConfig enemy_config;
    enemy_config.color_rect.width = ENEMY_SIZE;
    enemy_config.color_rect.height = ENEMY_SIZE;
    enemy_config.collider.width = ENEMY_SIZE;
    enemy_config.collider.height = ENEMY_SIZE;

    for (int i = 0; i < ENEMY_QUANTITY; ++i) {
        // Posições aleatórias dentro do mapa
        float x = static_cast<float>(rand()) / RAND_MAX * MAP_WIDTH;
        float y = static_cast<float>(rand()) / RAND_MAX * MAP_HEIGHT;

        enemy_config.transform.position = {x, y};

        // Cores aleatórias (R,G,B) entre 100 e 255 para evitar cores muito escuras
        enemy_config.color_rect.color.r = 100 + rand() % 156;
        enemy_config.color_rect.color.g = 100 + rand() % 156;
        enemy_config.color_rect.color.b = 100 + rand() % 156;
        enemy_config.color_rect.color.a = 255;

        create_enemy(registry, enemy_config);
    }
}
