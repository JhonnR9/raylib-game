// Created by jhone on 13/08/2025.
//

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <raylib.h>
#include <unordered_set>
#include <entt/entity/entity.hpp> // Necessário para entt::entity
#include <utility>

namespace rpg {

    struct ColorRect {
        Color color;
        float width;
        float height;

        ColorRect() = default;
        ColorRect(Color c, float w, float h)
            : color(c), width(w), height(h) {}
    };

    struct Input {
        Vector2 move_direction;

        Input() = default;
        explicit Input(const Vector2 dir) : move_direction(dir) {}
    };

    struct Sprite {
        std::string name;
        mutable Vector2 size{};
        Color color{};
        Sprite() = default;

        explicit Sprite(std::string p, const Vector2 s, const Color c) : name(std::move(p)) , size(s) , color(c) {

        }
    };

    struct Transform {
        Vector2 position;
        float rotation;
        Vector2 scale;

        Transform() = default;
        Transform(Vector2 p, float r, Vector2 s)
            : position(p), rotation(r), scale(s) {}
    };

    struct BoxCollider2D {
        float width = 0;
        float height = 0;
        bool is_colliding = false;
        bool is_trigger = false;
        bool is_static = false;
        std::unordered_set<entt::entity> colliding_entities;

        BoxCollider2D() = default;
        BoxCollider2D(float w, float h, bool colliding, bool trigger, bool stat)
            : width(w), height(h), is_colliding(colliding),
              is_trigger(trigger), is_static(stat) {}
    };

    struct MovementData {
        Vector2 velocity;
        float speed;
        Vector2 previous_position;

        MovementData() = default;
        MovementData(Vector2 vel, float spd, Vector2 prev_pos)
            : velocity(vel), speed(spd), previous_position(prev_pos) {}
    };

} // namespace rpg

#endif // COMPONENTS_H
