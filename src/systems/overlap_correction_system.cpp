#include "overlap_correction_system.h"
#include <iostream>
#include <cmath>
#include <execution>
#include <vector>

namespace rpg {

    OverlapCorrectionSystem::OverlapCorrectionSystem(entt::registry *registry)
        : System(registry) {}

    // ---------------------- Helper Functions ----------------------

    // Calculates overlap between two colliders based on their transforms
    OverlapResult OverlapCorrectionSystem::calculate_overlap(const CollisionContext &ctx) {
        const float half_a_w = ctx.colliderA.width * 0.5f;
        const float half_a_h = ctx.colliderA.height * 0.5f;
        const float half_b_w = ctx.colliderB.width * 0.5f;
        const float half_b_h = ctx.colliderB.height * 0.5f;

        const Vector2 delta = {
            ctx.transformA.position.x - ctx.transformB.position.x,
            ctx.transformA.position.y - ctx.transformB.position.y
        };

        const float overlap_x = (half_a_w + half_b_w) - std::abs(delta.x);
        const float overlap_y = (half_a_h + half_b_h) - std::abs(delta.y);

        return {delta, overlap_x, overlap_y};
    }

    // Determines the minimum translation vector to separate overlapping colliders
    Vector2 OverlapCorrectionSystem::compute_correction(const OverlapResult &overlap) {
        if (overlap.x < overlap.y) {
            float sign_x = (overlap.delta.x > 0) ? 1.0f : -1.0f;
            return {overlap.x * sign_x * 0.5f, 0.0f};
        } else {
            float sign_y = (overlap.delta.y > 0) ? 1.0f : -1.0f;
            return {0.0f, overlap.y * sign_y * 0.5f};
        }
    }

    // Applies calculated position corrections and resets velocity on corrected axes
    void OverlapCorrectionSystem::apply_corrections(
        const std::unordered_map<entt::entity, CorrectionIntent> &corrections,
        float epsilon
    ) {
        for (auto &[entity, intent] : corrections) {
            if (!registry->valid(entity)) continue;
            auto &transform = registry->get<Transform>(entity);
            auto &movement = registry->get<MovementData>(entity);

            if (std::abs(intent.offset.x) > epsilon ||
                std::abs(intent.offset.y) > epsilon) {
                transform.position.x += intent.offset.x;
                transform.position.y += intent.offset.y;

                if (std::abs(intent.offset.x) > epsilon) movement.velocity.x = 0.0f;
                if (std::abs(intent.offset.y) > epsilon) movement.velocity.y = 0.0f;
            }
        }
    }

    // ---------------------- Main Update Loop ----------------------

    void OverlapCorrectionSystem::run(float dt) {
        constexpr int MAX_ITERATIONS = 10;
        constexpr float EPSILON = 0.01f;

        bool has_overlap = true;
        int iteration = 0;

        while (has_overlap && iteration < MAX_ITERATIONS) {
            has_overlap = false;

            // Step 1: Gather all colliding pairs
            std::vector<std::pair<entt::entity, entt::entity>> pairs;
            {
                auto view = registry->view<BoxCollider2D, Transform>();
                for (auto [entity, collider, transform] : view.each()) {
                    if (!collider.is_colliding) continue;

                    for (auto other : collider.colliding_entities) {
                        auto pair = std::minmax(entity, other);
                        pairs.emplace_back(pair);
                    }
                }
            }

            // Step 2: Compute corrections in parallel and merge results
            auto merged_corrections = std::transform_reduce(
                std::execution::par,
                pairs.begin(),
                pairs.end(),
                std::unordered_map<entt::entity, CorrectionIntent>{},

                // Merge function to accumulate correction intents
                [](auto a, const auto &b) {
                    for (auto &[entity, intent] : b) {
                        a[entity].offset.x += intent.offset.x;
                        a[entity].offset.y += intent.offset.y;
                    }
                    return a;
                },

                // Local correction calculation per colliding pair
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

                    CollisionContext ctx{entityA, entityB, colliderA, colliderB, transformA, transformB};
                    auto overlap = calculate_overlap(ctx);

                    if (!overlap.has_penetration()) return local;

                    has_overlap = true;

                    Vector2 correction = compute_correction(overlap);

                    local[entityA].offset.x += correction.x;
                    local[entityA].offset.y += correction.y;
                    local[entityB].offset.x -= correction.x;
                    local[entityB].offset.y -= correction.y;

                    return local;
                }
            );

            // Step 3: Apply accumulated corrections to entities
            apply_corrections(merged_corrections, EPSILON);

            iteration++;
        }
    }

} // namespace rpg
