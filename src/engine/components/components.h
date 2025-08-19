// Created by jhone on 13/08/2025.
//

#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <raylib.h>
#include <string>
#include <unordered_set>
#include <utility>

#include "entt/entt.hpp"

namespace rpg {

    struct ColorRect {
        Color color;
        float width;
        float height;

        ColorRect() = default;
        ColorRect(Color color, float width, float height)
            : color(color), width(width), height(height) {}
    };

    struct Input {
        Vector2 move_direction;

        Input() = default;
        explicit Input(const Vector2 move_direction)
            : move_direction(move_direction) {}
    };

    struct Sprite {
        std::string name;
        mutable Vector2 size{};
        Color color{};
        Sprite() = default;

        explicit Sprite(std::string name, const Vector2 size, const Color color)
            : name(std::move(name)), size(size), color(color) {}
    };

    struct Transform {
        Vector2 position;
        float rotation;
        Vector2 scale;

        Transform() = default;
        Transform(Vector2 position, float rotation, Vector2 scale)
            : position(position), rotation(rotation), scale(scale) {}
    };

    struct BoxCollider2D {
        float width = 0;
        float height = 0;
        bool is_colliding = false;
        bool is_trigger = false;
        bool is_static = false;
        bool sync_size_with_sprite = true;
        std::unordered_set<entt::entity> colliding_entities;

        BoxCollider2D() = default;
        BoxCollider2D(float width, float height, bool is_colliding, bool is_trigger, bool is_static, bool sync_size_with_sprite)
            : width(width), height(height), is_colliding(is_colliding),
              is_trigger(is_trigger), is_static(is_static), sync_size_with_sprite(sync_size_with_sprite) {}
    };

    struct MovementData {
        Vector2 velocity;
        float speed;
        Vector2 previous_position;

        MovementData() = default;
        MovementData(Vector2 velocity, float speed, Vector2 previous_position)
            : velocity(velocity), speed(speed), previous_position(previous_position) {}
    };

} // namespace rpg

#endif // COMPONENTS_H
