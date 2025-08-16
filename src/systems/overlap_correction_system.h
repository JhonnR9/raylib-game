// Created by jhone on 16/08/2025.

#ifndef OVERLAP_CORRECTION_SYSTEM_H
#define OVERLAP_CORRECTION_SYSTEM_H

#include "system.h"
#include <unordered_map>
#include <vector>
#include <set>
#include "components/components.h"

namespace rpg {

    struct CollisionContext {
        entt::entity entity_a;
        entt::entity entity_b;
        BoxCollider2D &collider_a;
        BoxCollider2D &collider_b;
        Transform &transform_a;
        Transform &transform_b;
    };

    struct OverlapResult {
        Vector2 delta;
        float x;
        float y;

        [[nodiscard]] bool has_penetration() const {
            return x > 0 && y > 0;
        }
    };

    struct CorrectionIntent {
        Vector2 offset{0, 0};
    };

    class OverlapCorrectionSystem final : public System {
    public:
        explicit OverlapCorrectionSystem(entt::registry* registry);
        void run(float delta_time) override;

    private:
        // Core steps separated into functions to ease maintenance
        [[nodiscard]] std::set<std::pair<entt::entity, entt::entity>> collect_overlapping_pairs() const;
        std::unordered_map<entt::entity, CorrectionIntent> calculate_all_corrections(
            const std::set<std::pair<entt::entity, entt::entity>>& pairs);
        void apply_corrections(const std::unordered_map<entt::entity, CorrectionIntent>& corrections);
        [[nodiscard]] bool check_any_overlap() const;

        // Helper functions for individual calculation
        static OverlapResult calculate_overlap(const CollisionContext& ctx);
        static Vector2 compute_correction(const OverlapResult& overlap);
        static void accumulate_correction(std::unordered_map<entt::entity, CorrectionIntent>& corrections,
                                          entt::entity entity, const Vector2& offset);

    private:
        static constexpr int MAX_ITERATIONS = 3;
        static constexpr float EPSILON = 0.001f;
    };

} // namespace rpg

#endif // OVERLAP_CORRECTION_SYSTEM_H
