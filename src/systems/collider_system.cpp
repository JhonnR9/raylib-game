#include "collider_system.h"

#include <iostream>
#include <set>
#include <cmath>

#include "../components/components.h"

namespace rpg {

    ColliderSystem::ColliderSystem(entt::registry* registry)
        : System(registry), hash_grid_cells() {}

    std::pair<int, int> ColliderSystem::get_hash_grid_cell(float x, float y) const {
        return {
            static_cast<int>(std::floor(x / hash_grid_cell_size)),
            static_cast<int>(std::floor(y / hash_grid_cell_size))
        };
    }

    void ColliderSystem::register_pointer(entt::entity entity, const Vector2& position) {
        auto cell = get_hash_grid_cell(position.x, position.y);
        hash_grid_cells[cell].push_back(entity);
    }

    std::vector<entt::entity> ColliderSystem::get_nearby_entities(const Vector2& position) {
        std::vector<entt::entity> result;
        auto base = get_hash_grid_cell(position.x, position.y);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                auto cell = std::make_pair(base.first + dx, base.second + dy);
                auto it = hash_grid_cells.find(cell);
                if (it != hash_grid_cells.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }

        return result;
    }

    void ColliderSystem::run(float dt) {
        hash_grid_cells.clear();
        std::set<std::pair<int, int>> pairs_checked;

        auto view = registry->view<BoxCollider2D, Transform>();

        // Preenche o grid com as entidades
        for (auto [entity, collider, transform] : view.each()) {
            register_pointer(entity, transform.position);
        }

        // Verifica colisões apenas com vizinhos próximos
        for (auto [entity1, collider1, transform1] : view.each()) {
            auto nearby_entities = get_nearby_entities(transform1.position);

            for (auto entity2 : nearby_entities) {
                if (entity1 == entity2) continue;

                auto pair = std::minmax((int)entity1, (int)entity2);
                if (pairs_checked.contains(pair)) continue;
                pairs_checked.insert(pair);

                auto& collider2 = registry->get<BoxCollider2D>(entity2);
                auto& transform2 = registry->get<Transform>(entity2);

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

} // namespace rpg
