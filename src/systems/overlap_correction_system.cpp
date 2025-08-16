#include "overlap_correction_system.h"
#include <iostream>
#include <execution>
#include <set>
#include <vector>

#include "raymath.h"


namespace rpg {
    OverlapCorrectionSystem::OverlapCorrectionSystem(entt::registry *registry)
        : System(registry) {
    }

    // ---------------------- Helper Functions ----------------------

    // Calculates overlap between two colliders based on their transforms
    OverlapResult OverlapCorrectionSystem::calculate_overlap(const CollisionContext &ctx) {
        const float half_a_w = ctx.collider_a.width * 0.5f;
        const float half_a_h = ctx.collider_a.height * 0.5f;
        const float half_b_w = ctx.collider_b.width * 0.5f;
        const float half_b_h = ctx.collider_b.height * 0.5f;

        const Vector2 delta = {
            ctx.transform_a.position.x - ctx.transform_b.position.x,
            ctx.transform_a.position.y - ctx.transform_b.position.y
        };

        const float overlap_x = (half_a_w + half_b_w) - std::abs(delta.x);
        const float overlap_y = (half_a_h + half_b_h) - std::abs(delta.y);

        return {delta, overlap_x, overlap_y};
    }

    // Computes the minimum translation vector (MTV) to resolve overlap between two colliders,
    // moving each one halfway along the axis of least penetration.
    Vector2 OverlapCorrectionSystem::compute_correction(const OverlapResult &overlap) {
        if (overlap.x <= 0.0f || overlap.y <= 0.0f) {
            return {0.0f, 0.0f};
        }

        if (overlap.x < overlap.y) {
            const float sign_x = (overlap.delta.x > 0) ? 1.0f : -1.0f;
            return {overlap.x * sign_x * 0.5f, 0.0f};
        } else {
            const float sign_y = (overlap.delta.y > 0) ? 1.0f : -1.0f;
            return {0.0f, overlap.y * sign_y * 0.5f};
        }
    }

    // Applies position corrections to entities
    // if the correction exceeds a minimal threshold (epsilon).
    void OverlapCorrectionSystem::apply_corrections(
        const std::unordered_map<entt::entity, CorrectionIntent> &corrections) {
        for (auto &[entity, intent]: corrections) {
            constexpr float MY_EPSILON = .001f;

            if (!registry->valid(entity)) continue;
            auto &transform = registry->get<Transform>(entity);

            const bool correct_x = std::abs(intent.offset.x) > MY_EPSILON;
            const bool correct_y = std::abs(intent.offset.y) > MY_EPSILON;

            if (correct_x || correct_y) {
                transform.position = Vector2Add(transform.position, intent.offset);
            }
        }
    }

    void OverlapCorrectionSystem::run(float dt) {
    constexpr int MAX_ITERATIONS = 3;

    bool has_overlap = true;
    int iteration = 0;

    while (has_overlap && iteration < MAX_ITERATIONS) {
        // Step 1: Collect unique overlapping entity pairs
        std::set<std::pair<entt::entity, entt::entity>> unique_pairs;
        {
            auto view = registry->view<BoxCollider2D, Transform>();
            for (auto [entity, collider, transform] : view.each()) {
                if (!collider.is_colliding || collider.is_trigger) continue;

                for (auto other : collider.colliding_entities) {
                    auto pair = std::minmax(entity, other);
                    unique_pairs.insert(pair);
                }
            }
        }

        // Step 2: Compute corrections in parallel
        std::vector pairs_vec(unique_pairs.begin(), unique_pairs.end());

        auto merged_corrections = std::transform_reduce(
            std::execution::par,
            pairs_vec.begin(),
            pairs_vec.end(),
            std::unordered_map<entt::entity, CorrectionIntent>{},

            // Merge function
            [](auto a, const auto &b) {
                for (auto &[entity, intent] : b) {
                    a[entity].offset.x += intent.offset.x;
                    a[entity].offset.y += intent.offset.y;
                }
                return a;
            },

            // Mapping function
            [&](const auto &pair) {
                std::unordered_map<entt::entity, CorrectionIntent> local;

                auto entityA = pair.first;
                auto entityB = pair.second;

                if (!registry->valid(entityA) || !registry->valid(entityB))
                    return local;

                auto &colliderA = registry->get<BoxCollider2D>(entityA);
                auto &colliderB = registry->get<BoxCollider2D>(entityB);
                auto &transformA = registry->get<Transform>(entityA);
                auto &transformB = registry->get<Transform>(entityB);

                const CollisionContext ctx{entityA, entityB, colliderA, colliderB, transformA, transformB};
                const auto overlap = calculate_overlap(ctx);

                if (!overlap.has_penetration()) return local;

                Vector2 correction = compute_correction(overlap);

                local[entityA].offset.x += correction.x;
                local[entityA].offset.y += correction.y;
                local[entityB].offset.x -= correction.x;
                local[entityB].offset.y -= correction.y;

                return local;
            }
        );

        // Step 3: Apply all position corrections
        apply_corrections(merged_corrections);

        // Step 4: Check if there are still overlaps after corrections
        has_overlap = false;
        {
            auto view = registry->view<BoxCollider2D, Transform>();
            for (auto [entity, collider, transform] : view.each()) {
                if (!collider.is_colliding || collider.is_trigger) continue;

                for (const auto other : collider.colliding_entities) {
                    if (!registry->valid(entity) || !registry->valid(other)) continue;

                    auto &colliderA = registry->get<BoxCollider2D>(entity);
                    auto &colliderB = registry->get<BoxCollider2D>(other);
                    auto &transformA = registry->get<Transform>(entity);
                    auto &transformB = registry->get<Transform>(other);

                    const CollisionContext ctx{entity, other, colliderA, colliderB, transformA, transformB};
                    const auto overlap = calculate_overlap(ctx);

                    if (overlap.has_penetration()) {
                        has_overlap = true;
                        break;
                    }
                }
                if (has_overlap) break;
            }
        }

        iteration++;
    }
}

} // namespace rpg
