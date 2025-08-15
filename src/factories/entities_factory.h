#ifndef ENTITIES_FACTORY_H
#define ENTITIES_FACTORY_H

#include "entt/entity/registry.hpp"
#include "components/components.h"

namespace rpg {

    struct PlayerConfig {
        ColorRect color_rect{ Color(30, 200, 25, 255), 50.f, 50.f };
        Transform transform{ {0.f, 0.f}, 0.f, {1.f, 1.f} };
        Input input{ {0.f, 0.f} };
        BoxCollider2D collider{ 50.f, 50.f, false, false, false };
        MovementData movement_data{ {0.f, 0.f}, 300.f, {0.f, 0.f} };
    };

    void create_player(entt::registry* registry, const PlayerConfig& config);

    struct EnemyConfig {
        ColorRect color_rect{ Color(138, 73, 51, 255), 100.f, 100.f };
        Transform transform{ {0.f, 0.f}, 0.f, {1.f, 1.f} };
        Input input{ {0.f, 0.f} };
        BoxCollider2D collider{ 100.f, 100.f, false, false, true };
        MovementData movement_data{ {0.f, 0.f}, 300.f, {0.f, 0.f} };
    };

    void create_enemy(entt::registry* registry, const EnemyConfig& config);

} // namespace rpg

#endif // ENTITIES_FACTORY_H
