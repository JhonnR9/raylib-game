// Created by jhone on 14/08/2025.
//

#ifndef OVERLAP_CORRECTION_SYSTEM_H
#define OVERLAP_CORRECTION_SYSTEM_H
#include <optional>

#include "system.h"
#include <unordered_set>
#include "components/components.h"

namespace rpg {
    // Estrutura representando contexto de colisão entre duas entidades
    struct CollisionContext {
        entt::entity entityA;
        entt::entity entityB;
        BoxCollider2D &colliderA;
        BoxCollider2D &colliderB;
        Transform &transformA;
        Transform &transformB;
    };

    //Overlap calculation result
    struct OverlapResult {
        Vector2 delta; // Distance between centers
        float x; // Overlay on the X axis
        float y; // Overlay on the Y axis

        [[nodiscard]] bool has_penetration() const {
            return x > 0 && y > 0;
        }
    };

    // System responsible for resolving physical overlap between 2D entities.
    class OverlapCorrectionSystem final : public System {
        // Symmetric hash for pairs of entities
        struct EntityPairHash {
            std::size_t operator()(const std::pair<entt::entity, entt::entity> &pair) const;
        };

        // Symmetric equality between pairs of entities
        struct EntityPairEqual {
            bool operator()(const std::pair<entt::entity, entt::entity> &lhs,
                            const std::pair<entt::entity, entt::entity> &rhs) const;
        };

        // Avoid processing the same pair twice in the same frame
        std::unordered_set<std::pair<entt::entity, entt::entity>, EntityPairHash, EntityPairEqual>
        processed_pairs;

    public:
        explicit OverlapCorrectionSystem(entt::registry *registry);

        void run(float delta_time) override;

    private:
        // --- Main Helpers ---

        static bool should_skip_entity(const BoxCollider2D &collider);

        bool has_already_processed(entt::entity a, entt::entity b);

        static OverlapResult calculate_overlap(const CollisionContext &ctx);

        static void resolve_overlap(const OverlapResult &overlap,
                                    Transform &movable_transform,
                                    MovementData &movable_movement);

        static void resolve_overlap_between(
            const OverlapResult &overlap,
            Transform &transformA, MovementData &movementA,
            Transform &transformB, MovementData &movementB);

        std::optional<std::pair<Transform &, MovementData &> >
        choose_movable_entity(const CollisionContext &ctx) const;

        static void draw_debug(const CollisionContext &ctx);
    };
} // namespace rpg

#endif // OVERLAP_CORRECTION_SYSTEM_H
