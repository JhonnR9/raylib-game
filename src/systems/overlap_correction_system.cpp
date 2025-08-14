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
        const auto entity_view = registry->view<BoxCollider2D, Transform>();
        for (auto [entity_id, collider, transform]: entity_view.each()) {
            if (collider.is_colliding) {
                DrawRectangleLines(
                    static_cast<int>(transform.position.x - collider.width / 2),
                    static_cast<int>(transform.position.y - collider.height / 2),
                    static_cast<int>(collider.width),
                    static_cast<int>(collider.height),
                    PURPLE);
                std::cout << "Collision detected" << std::endl;
            }
        }
    }
} // rpg
