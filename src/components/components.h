//
// Created by jhone on 13/08/2025.
//

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <raylib.h>
#include <unordered_set>

namespace rpg {
    struct ColorRect {
        Color color;
        float width;
        float height;
    };

    struct Input {
        Vector2 move_direction;
    };

    struct Sprite {
        Texture2D *texture;
        Vector2 uv;
        Vector2 size;
    };

    struct Transform {
        Vector2 position;
        float rotation;
        Vector2 scale;
    };

    struct BoxCollider2D {
        float width = 0;
        float height = 0;
        bool is_colliding = false;
        bool is_trigger = false;
        bool is_static = false;
        std::unordered_set<entt::entity> colliding_entities;
    };

    struct MovementData {
        Vector2 velocity;
        float speed;
        Vector2 previous_position;
    };
} // rpg

#endif //COMPONENTS_H
