#ifndef ENTITIES_FACTORY_H
#define ENTITIES_FACTORY_H
#include "engine/components/components.h"
namespace rpg {

    struct PlayerConfig {
        //ColorRect color_rect{ Color(30, 200, 25, 255), 50.f, 50.f };
        Sprite sprite{"player.png", Vector2{10,10}, RAYWHITE };
        Transform transform{ {0.f, 0.f}, 0.f, {1.f, 1.f} };
        Input input{ {0.f, 0.f} };
        BoxCollider2D collider{ 30.f, 30.f, false, false, false, true };
        MovementData movement_data{ {0.f, 0.f}, 300.f, {0.f, 0.f} };
    };

    void create_player(entt::registry* registry, const PlayerConfig& config);

    struct EnemyConfig {
        Sprite sprite{"enemy.png", Vector2{10,10}, RAYWHITE };
        Transform transform{ {0.f, 0.f}, 0.f, {1.f, 1.f} };
        BoxCollider2D collider{ 60.f, 60.f, false, false, true, true };
        MovementData movement_data{ {0.f, 0.f}, 300.f, {0.f, 0.f} };
    };

    void create_enemy(entt::registry* registry, const EnemyConfig& config);

} // namespace rpg

#endif // ENTITIES_FACTORY_H
