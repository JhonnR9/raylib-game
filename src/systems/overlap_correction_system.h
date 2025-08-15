// Created by jhone on 14/08/2025.
//

#ifndef OVERLAP_CORRECTION_SYSTEM_H
#define OVERLAP_CORRECTION_SYSTEM_H
#include "system.h"
#include <unordered_set>
#include "../components/components.h"

namespace rpg {
    // System responsible for resolving physical overlaps between 2D entities.
    class OverlapCorrectionSystem final : public System {
        // Hash function for entity pairs, order-independent.
        struct EntityPairHash {
            std::size_t operator()(const std::pair<entt::entity, entt::entity> &pair) const;
        };

        // Equality check for entity pairs, order-independent.
        struct EntityPairEqual {
            bool operator()(
                const std::pair<entt::entity, entt::entity> &lhs,
                const std::pair<entt::entity, entt::entity> &rhs
            ) const;
        };

        // Tracks processed entity pairs to avoid double-resolving collisions in the same frame.
        std::unordered_set<std::pair<entt::entity, entt::entity>, EntityPairHash, EntityPairEqual>
        processed_pairs;

    public:
        explicit OverlapCorrectionSystem(entt::registry *registry);

        void run(float delta_time) override;

    private:
        // Skips entities that should not be part of collision resolution.
        static bool should_skip_entity(const BoxCollider2D &collider);

        // Checks if a given pair of entities has already been processed this frame.
        bool is_pair_already_processed(entt::entity entity_a, entt::entity entity_b);

        // Calculates the overlap distances on X and Y axes between two BoxColliders.
        static std::pair<float, float> compute_overlap(
            const BoxCollider2D &collider_a, const Transform &transform_a,
            const BoxCollider2D &collider_b, const Transform &transform_b);

        // Selects which entity should be moved to resolve overlap.
        bool select_movable_entity(
            entt::entity entity_a, entt::entity entity_b,
            const BoxCollider2D &collider_a,
            const BoxCollider2D &collider_b,
            Transform *&movable_transform,
            MovementData *&movable_movement
        ) const;

        // Adjusts position and zeroes velocity on the axis of resolution.
        static void apply_overlap_resolution(
            Vector2 delta, float overlap_x, float overlap_y,
            Transform *transform, MovementData *movement
        );

        // Debug helper: draws the collider outline.
        static void draw_debug_collider(const Transform &transform, const BoxCollider2D &collider);
    };
} // namespace rpg

#endif // OVERLAP_CORRECTION_SYSTEM_H
