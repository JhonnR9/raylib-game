// Created by jhone on 16/08/2025.
//

#ifndef OVERLAP_CORRECTION_SYSTEM_H
#define OVERLAP_CORRECTION_SYSTEM_H

#include "system.h"
#include <unordered_map>
#include "components/components.h"

namespace rpg {
    // Represents collision context between two entities
    struct CollisionContext {
        entt::entity entityA;
        entt::entity entityB;
        BoxCollider2D &colliderA;
        BoxCollider2D &colliderB;
        Transform &transformA;
        Transform &transformB;
    };

    // Overlap calculation result
    struct OverlapResult {
        Vector2 delta; // Vector between entity centers
        float x;       // Overlap amount on X axis
        float y;       // Overlap amount on Y axis

        [[nodiscard]] bool has_penetration() const {
            return x > 0 && y > 0;
        }
    };

    // Accumulated correction intent per entity
    struct CorrectionIntent {
        Vector2 offset{0, 0};
    };

    // System responsible for resolving 2D physical overlap between entities
    class OverlapCorrectionSystem final : public System {
    public:
        explicit OverlapCorrectionSystem(entt::registry *registry);

        void run(float delta_time) override;

    private:
        // Helper functions
        static OverlapResult calculate_overlap(const CollisionContext &ctx);
        static Vector2 compute_correction(const OverlapResult &overlap);

        void apply_corrections(
            const std::unordered_map<entt::entity, CorrectionIntent> &corrections,
            float epsilon
        );
    };
} // namespace rpg

#endif // OVERLAP_CORRECTION_SYSTEM_H
