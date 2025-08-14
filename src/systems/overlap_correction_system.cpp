//
// Created by jhone on 14/08/2025.
//

#include "overlap_correction_system.h"

#include <iostream>

#include "../components/components.h"

namespace rpg {
    OverlapCorrectionSystem::OverlapCorrectionSystem(entt::registry *registry): System(registry) {
    }

    void OverlapCorrectionSystem::run(float dt) {
    const auto entity_view = registry->view<BoxCollider2D, Transform, MovementData>();

    for (auto [entity_id, collider, transform, movement_data] : entity_view.each()) {
        if (!collider.is_colliding || collider.is_trigger || collider.colliding_entities.empty())
            continue;

        for (auto other_entity : collider.colliding_entities) {
            auto &other_collider = registry->get<BoxCollider2D>(other_entity);
            auto &other_transform = registry->get<Transform>(other_entity);

            float half_w_a = collider.width * 0.5f;
            float half_h_a = collider.height * 0.5f;
            float half_w_b = other_collider.width * 0.5f;
            float half_h_b = other_collider.height * 0.5f;

            Vector2 delta = {
                transform.position.x - other_transform.position.x,
                transform.position.y - other_transform.position.y
            };

            float overlap_x =  (half_w_a + half_w_b) - abs(delta.x);
            float overlap_y = (half_h_a + half_h_b) - abs(delta.y);

            if (overlap_x > 0 && overlap_y > 0) {
                if (overlap_x < overlap_y) {
                    float sign = (delta.x > 0) ? 1.0f : -1.0f;
                    transform.position.x += overlap_x * sign;

                    movement_data.velocity.x = 0.0f;
                } else {
                    float sign = (delta.y > 0) ? 1.0f : -1.0f;
                    transform.position.y += overlap_y * sign;

                    movement_data.velocity.y = 0.0f;
                }
            }
        }

        // Debug visual
        DrawRectangleLines(
            static_cast<int>(transform.position.x - collider.width / 2),
            static_cast<int>(transform.position.y - collider.height / 2),
            static_cast<int>(collider.width),
            static_cast<int>(collider.height),
            PURPLE
        );
    }
}

} // rpg
