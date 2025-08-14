#include "collision_detection_system.h"

#include <iostream>
#include <cmath>
#include <execution>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include "../components/components.h"

namespace rpg {
    CollisionDetectionSystem::CollisionDetectionSystem(entt::registry *registry)
        : System(registry) {
    }

    std::pair<int, int> CollisionDetectionSystem::get_hash_grid_cell(const float x, const float y) const {
        return {
            static_cast<int>(std::floor(x / hash_grid_cell_size)),
            static_cast<int>(std::floor(y / hash_grid_cell_size))
        };
    }

    void CollisionDetectionSystem::register_entity_in_grid(const entt::entity entity, const Vector2 &position) {
        const auto cell = get_hash_grid_cell(position.x, position.y);
        hash_grid_cells[cell].push_back(entity);
    }

    std::vector<entt::entity> CollisionDetectionSystem::get_nearby_entities(const Vector2 &position) {
        std::vector<entt::entity> nearby_entities;
        const auto base_cell = get_hash_grid_cell(position.x, position.y);
        const auto [first, second] = base_cell;

        // Loop through surrounding cells (3x3 neighborhood)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                auto cell = std::make_pair(first + dx, second + dy);
                if (auto it = hash_grid_cells.find(cell); it != hash_grid_cells.end()) {
                    // Append all entities from the cell to the nearby cache
                    nearby_entities.insert(nearby_entities.end(), it->second.begin(), it->second.end());
                }
            }
        }

        return nearby_entities;
    }

    void CollisionDetectionSystem::run(float dt) {
        hash_grid_cells.clear();
        std::unordered_set<std::pair<int, int>, pair_hash> processed_pairs;

        const auto entity_view = registry->view<BoxCollider2D, Transform>();

        // Populate the grid on the main thread
        for (auto [entity_id, box_collider, transform]: entity_view.each()) {
            register_entity_in_grid(entity_id, transform.position);
            box_collider.is_colliding = false;
            box_collider.colliding_entities.clear();
        }

        // List of all cells
        std::vector<std::pair<int, int>> all_cells;
        all_cells.reserve(hash_grid_cells.size());
        for (auto &kv: hash_grid_cells) {
            all_cells.push_back(kv.first);
        }

        // Local results for each thread
        struct CollisionResult {
            std::unordered_set<std::pair<int, int>, pair_hash> pairs;
        };
        std::vector<CollisionResult> thread_results(std::thread::hardware_concurrency());

        // Parallel loop
        std::for_each(
            std::execution::par_unseq,
            all_cells.begin(),
            all_cells.end(),
            [&](const auto &cell_coord) {
                const auto thread_id = std::hash<std::thread::id>()(std::this_thread::get_id()) % thread_results.size();
                auto &local = thread_results[thread_id];

                const auto &entities_in_cell = hash_grid_cells[cell_coord];

                for (auto entity_a_id: entities_in_cell) {
                    auto &transform_a = registry->get<Transform>(entity_a_id);
                    auto &collider_a = registry->get<BoxCollider2D>(entity_a_id);

                    auto nearby_entities = get_nearby_entities(transform_a.position);

                    for (auto entity_b_id: nearby_entities) {
                        if (entity_a_id == entity_b_id) continue;

                        auto entity_pair = std::minmax(
                            static_cast<int>(entity_a_id),
                            static_cast<int>(entity_b_id)
                        );

                        if (local.pairs.contains(entity_pair)) continue;
                        local.pairs.insert(entity_pair);

                        auto &transform_b = registry->get<Transform>(entity_b_id);
                        auto &collider_b = registry->get<BoxCollider2D>(entity_b_id);

                        // AABB collision
                        float a_min_x = transform_a.position.x - (collider_a.width / 2);
                        float a_min_y = transform_a.position.y - (collider_a.height / 2);
                        float b_min_x = transform_b.position.x - (collider_b.width / 2);
                        float b_min_y = transform_b.position.y - (collider_b.height / 2);

                        bool is_colliding =
                                a_min_x < b_min_x + collider_b.width &&
                                a_min_x + collider_a.width > b_min_x &&
                                a_min_y < b_min_y + collider_b.height &&
                                a_min_y + collider_a.height > b_min_y;

                        if (is_colliding) {
                            // We still need to protect modifications to the registry
                            static std::mutex reg_mutex;
                            std::scoped_lock lock(reg_mutex);
                            collider_a.is_colliding = true;
                            collider_b.is_colliding = true;
                            collider_a.colliding_entities.insert(entity_b_id);
                            collider_b.colliding_entities.insert(entity_a_id);
                        }
                    }
                }
            }
        );

        // Merge pairs
        for (auto &res: thread_results) {
            processed_pairs.insert(res.pairs.begin(), res.pairs.end());
        }

        hash_grid_cells.clear();
    }
} // namespace rpg
