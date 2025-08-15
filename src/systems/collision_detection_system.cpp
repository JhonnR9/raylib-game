#include "collision_detection_system.h"

#include <iostream>
#include <cmath>
#include <execution>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <numeric>
#include "../components/components.h"

namespace rpg {

    CollisionDetectionSystem::CollisionDetectionSystem(entt::registry *registry)
        : System(registry) {
    }

    // Calculates the grid cell (x, y) in which a position falls
    std::pair<int, int> CollisionDetectionSystem::get_hash_grid_cell(const float x, const float y) const {
        return {
            static_cast<int>(std::floor(x / hash_grid_cell_size)),
            static_cast<int>(std::floor(y / hash_grid_cell_size))
        };
    }

    // Registers an entity into its corresponding spatial hash grid cell
    void CollisionDetectionSystem::register_entity_in_grid(const entt::entity entity, const Vector2 &position) {
        const auto cell = get_hash_grid_cell(position.x, position.y);
        hash_grid_cells[cell].push_back(entity);
    }

    // Collects all nearby entities within a 3x3 grid neighborhood
    std::vector<entt::entity> CollisionDetectionSystem::get_nearby_entities(const Vector2 &position) {
        std::vector<entt::entity> nearby_entities;
        const auto base_cell = get_hash_grid_cell(position.x, position.y);
        const auto [first, second] = base_cell;

        // Iterate over surrounding 3x3 grid cells
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                auto cell = std::make_pair(first + dx, second + dy);
                if (auto it = hash_grid_cells.find(cell); it != hash_grid_cells.end()) {
                    // Append all entities in the neighbor cell
                    nearby_entities.insert(nearby_entities.end(), it->second.begin(), it->second.end());
                }
            }
        }

        return nearby_entities;
    }

    // Populates the hash grid with entities and resets their collision state
    void CollisionDetectionSystem::populate_hash_grid_cells() {
        const auto entity_view = registry->view<BoxCollider2D, Transform>();

        for (auto [entity_id, box_collider, transform]: entity_view.each()) {
            register_entity_in_grid(entity_id, transform.position);
            box_collider.is_colliding = false; // Reset collision state
            box_collider.colliding_entities.clear();
        }
    }

    // Checks collisions between a set of entities and stores results in local_collision_result
    void CollisionDetectionSystem::check_collision(
        const std::vector<entt::entity> &entities,
        CollisionResult &local_collision_result
    ) {
        for (auto entity_a_id: entities) {
            auto &transform_a = registry->get<Transform>(entity_a_id);
            const auto &collider_a = registry->get<BoxCollider2D>(entity_a_id);

            std::vector<entt::entity> nearby_entities = get_nearby_entities(transform_a.position);

            for (auto entity_b_id: nearby_entities) {
                if (entity_a_id == entity_b_id) continue; // Skip self-collision

                // Sort entity pair to ensure consistent order (min, max)
                auto entity_pair = std::minmax(entity_a_id, entity_b_id);

                // Avoid duplicate collision checks
                if (local_collision_result.pairs.contains(entity_pair)) continue;

                const auto &transform_b = registry->get<Transform>(entity_b_id);
                const auto &collider_b = registry->get<BoxCollider2D>(entity_b_id);

                // Axis-Aligned Bounding Box (AABB) collision detection
                const float a_min_x = transform_a.position.x - (collider_a.width / 2);
                const float a_min_y = transform_a.position.y - (collider_a.height / 2);
                const float b_min_x = transform_b.position.x - (collider_b.width / 2);
                const float b_min_y = transform_b.position.y - (collider_b.height / 2);

                const bool is_colliding =
                        a_min_x < b_min_x + collider_b.width &&
                        a_min_x + collider_a.width > b_min_x &&
                        a_min_y < b_min_y + collider_b.height &&
                        a_min_y + collider_a.height > b_min_y;

                if (is_colliding) {
                    local_collision_result.pairs.insert(entity_pair);
                }
            }
        }
    }

    // Main update loop
    void CollisionDetectionSystem::run(float dt) {
        hash_grid_cells.clear();
        populate_hash_grid_cells();

        // Extract all occupied grid cells
        std::vector<std::pair<int, int>> all_cells;
        all_cells.reserve(hash_grid_cells.size());
        for (const auto &key: hash_grid_cells | std::views::keys) {
            all_cells.push_back(key);
        }

        // Perform parallel collision checks using transform_reduce
        CollisionResult merged_result = std::transform_reduce(
            std::execution::par,
            all_cells.begin(),
            all_cells.end(),
            CollisionResult{},

            // Merge local results into one
            [](CollisionResult a, const CollisionResult &b) {
                a.pairs.insert(b.pairs.begin(), b.pairs.end());
                return a;
            },

            // Check collisions in each cell
            [&](const auto &cell_coord) {
                CollisionResult local;

                if (auto it = hash_grid_cells.find(cell_coord); it != hash_grid_cells.end()) {
                    const auto &entities_in_cell = it->second;
                    check_collision(entities_in_cell, local);
                }
                return local;
            }
        );

        // Mark entities as colliding and store references
        std::unordered_set<std::pair<entt::entity, entt::entity>, EntityPairHash, EntityPairEqual> processed_pairs;
        for (auto &pair: merged_result.pairs) {
            if (!processed_pairs.contains(pair)) {
                processed_pairs.insert(pair);

                auto entity_a_id = pair.first;
                auto entity_b_id = pair.second;

                auto &collider_a = registry->get<BoxCollider2D>(entity_a_id);
                auto &collider_b = registry->get<BoxCollider2D>(entity_b_id);

                collider_a.is_colliding = true;
                collider_b.is_colliding = true;
                collider_a.colliding_entities.insert(entity_b_id);
                collider_b.colliding_entities.insert(entity_a_id);
            }
        }
    }

    std::size_t CollisionDetectionSystem::EntityPairHash::operator()(
     const std::pair<entt::entity, entt::entity> &pair) const {
        // Normalize entity order so hash is symmetric
        const entt::entity min_entity = std::min(pair.first, pair.second);
        const entt::entity max_entity = std::max(pair.first, pair.second);

        const size_t hash_min = std::hash<entt::entity>()(min_entity);
        const size_t hash_max = std::hash<entt::entity>()(max_entity);

        // Combine hashes using a variant of boost::hash_combine
        return hash_min ^ (hash_max + 0x9e3779b9 + (hash_min << 6) + (hash_min >> 2));
    }

    bool CollisionDetectionSystem::EntityPairEqual::operator()(const std::pair<entt::entity, entt::entity> &lhs,
                                                              const std::pair<entt::entity, entt::entity> &rhs) const {
        // Equal if both entities match, regardless of order
        return (lhs.first == rhs.first && lhs.second == rhs.second) ||
               (lhs.first == rhs.second && lhs.second == rhs.first);
    }

} // namespace rpg
