#ifndef COLLIDER_SYSTEM_H
#define COLLIDER_SYSTEM_H

#include "system.h"
#include <raylib.h>
#include <unordered_map>
#include <vector>
#include <utility>
#include <entt/entt.hpp>

namespace rpg {

    struct pair_hash {
        std::size_t operator()(const std::pair<int, int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };

    class CollisionDetectionSystem final : public System {
        float hash_grid_cell_size{200.0f};
        std::unordered_map<std::pair<int, int>, std::vector<entt::entity>, pair_hash> hash_grid_cells;

        std::pair<int, int> get_hash_grid_cell(float x, float y) const;
        void register_entity_in_grid(entt::entity entity, const Vector2& position);
        std::vector<entt::entity> get_nearby_entities(const Vector2& position);

    public:
        explicit CollisionDetectionSystem(entt::registry* registry);
        void run(float dt) override;
    };

} // namespace rpg

#endif // COLLIDER_SYSTEM_H
