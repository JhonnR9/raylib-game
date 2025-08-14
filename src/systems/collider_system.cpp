#include "collider_system.h"

#include <iostream>
#include <set>
#include <cmath>

#include "../components/components.h"

namespace rpg {
    ColliderSystem::ColliderSystem(entt::registry *registry)
        : System(registry) {
    }

    std::pair<int, int> ColliderSystem::get_hash_grid_cell(const float x, const float y) const {
        return {
            static_cast<int>(std::floor(x / hash_grid_cell_size)),
            static_cast<int>(std::floor(y / hash_grid_cell_size))
        };
    }

    void ColliderSystem::register_entity_in_grid(const entt::entity entity, const Vector2 &position) {
        const auto cell = get_hash_grid_cell(position.x, position.y);
        hash_grid_cells[cell].push_back(entity);
    }

    std::vector<entt::entity> &ColliderSystem::get_nearby_entities(const Vector2 &position) {
        nearby_cache.clear();
        const auto base_cell = get_hash_grid_cell(position.x, position.y);
        const auto [first, second] = base_cell;

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                auto cell = std::make_pair(first + dx, second + dy);
                if (auto it = hash_grid_cells.find(cell); it != hash_grid_cells.end()) {
                    nearby_cache.insert(nearby_cache.end(), it->second.begin(), it->second.end());
                }
            }
        }

        return nearby_cache;
    }

    void ColliderSystem::run(float dt) {
        hash_grid_cells.clear();
        std::unordered_set<std::pair<int, int>, pair_hash> processed_pairs;

        const auto entity_view = registry->view<BoxCollider2D, Transform>();

        // Fill the grid with entities
        for (auto [entity_id, box_collider, transform]: entity_view.each()) {
            register_entity_in_grid(entity_id, transform.position);
            box_collider.is_colliding = false;
            box_collider.colliding_entities.clear();
        }

        // Checks collisions only with nearby neighbors
        for (auto [entity_a_id, collider_a, transform_a]: entity_view.each()) {
            for (
                const auto nearby_entities = get_nearby_entities(transform_a.position);
                auto entity_b_id: nearby_entities
            ) {
                if (entity_a_id == entity_b_id) continue;

                auto entity_pair = std::minmax(static_cast<int>(entity_a_id), static_cast<int>(entity_b_id));
                if (processed_pairs.contains(entity_pair)) continue;

                processed_pairs.insert(entity_pair);

                auto &collider_b = registry->get<BoxCollider2D>(entity_b_id);
                const auto &transform_b = registry->get<Transform>(entity_b_id);

                const float collider_a_min_x = transform_a.position.x - (collider_a.width / 2);
                const float collider_a_min_y = transform_a.position.y - (collider_a.height / 2);
                const float collider_b_min_x = transform_b.position.x - (collider_b.width / 2);
                const float collider_b_min_y = transform_b.position.y - (collider_b.height / 2);

                const bool is_colliding =
                        collider_a_min_x < collider_b_min_x + collider_b.width &&
                        collider_a_min_x + collider_a.width > collider_b_min_x &&
                        collider_a_min_y < collider_b_min_y + collider_b.height &&
                        collider_a_min_y + collider_a.height > collider_b_min_y;

                if (is_colliding) {
                    collider_a.is_colliding = true;
                    collider_b.is_colliding = true;
                    collider_a.colliding_entities.insert(entity_b_id);
                    collider_b.colliding_entities.insert(entity_a_id);

                }

#ifdef BUILD_COLLISION_DEBUG_DRAW_MODE
                if (is_colliding) {
                    DrawRectangleLines(
                        static_cast<int>(collider_a_min_x),
                        static_cast<int>(collider_a_min_y),
                        static_cast<int>(collider_a.width),
                        static_cast<int>(collider_a.height),
                        PURPLE);
                    DrawRectangleLines(
                        static_cast<int>(collider_b_min_x),
                        static_cast<int>(collider_b_min_y),
                        static_cast<int>(collider_b.width),
                        static_cast<int>(collider_b.height),
                        PURPLE);
                }
#endif // BUILD_COLLISION_DEBUG_DRAW_MODE
            }
        }
        hash_grid_cells.clear();
    }

} // namespace rpg
