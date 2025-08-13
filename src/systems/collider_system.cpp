//
// Created by jhone on 13/08/2025.
//

#include "collider_system.h"

#include <iostream>
#include <set>

#include "../components/components.h"

namespace rpg {
    ColliderSystem::ColliderSystem(entt::registry *registry): System(registry) {
    }

    void ColliderSystem::run(float dt) {
        std::set<std::pair<int, int> > pairs_checked;

        auto view = registry->view<BoxCollider2D, Transform>();

        for (auto [entity1, collider1, transform1]: view.each()) {
            for (auto [entity2, collider2, transform2]: view.each()) {
                if (entity1 == entity2) continue;

                auto pair = std::minmax((int) entity1, (int) entity2);
                if (pairs_checked.contains(pair)) continue;
                pairs_checked.insert(pair);

                float ent1_x = transform1.position.x - (collider1.width / 2);
                float ent1_y = transform1.position.y - (collider1.height / 2);
                float ent2_x = transform2.position.x - (collider2.width / 2);
                float ent2_y = transform2.position.y - (collider2.height / 2);

                bool collision =
                        ent1_x < ent2_x + collider2.width &&
                        ent1_x + collider1.width > ent2_x &&
                        ent1_y < ent2_y + collider2.height &&
                        ent1_y + collider1.height > ent2_y;

                if (collision) {
                    DrawRectangleLines(ent1_x, ent1_y, collider1.width, collider1.height, RAYWHITE);
                    DrawRectangleLines(ent2_x, ent2_y, collider2.width, collider2.height, RED);
                }
            }
        }
    }
} // rpg
