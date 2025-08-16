#include "overlap_correction_system.h"
#include <execution>
#include <vector>
#include <algorithm>
#include "raymath.h"

namespace rpg {

OverlapCorrectionSystem::OverlapCorrectionSystem(entt::registry* registry)
    : System(registry) {}

// Collects unique pairs of entities that are currently colliding and are not triggers
std::set<std::pair<entt::entity, entt::entity>> OverlapCorrectionSystem::collect_overlapping_pairs() const {
    std::set<std::pair<entt::entity, entt::entity>> unique_pairs;

    auto view = registry->view<BoxCollider2D, Transform>();
    for (auto [entity, collider, transform] : view.each()) {
        if (!collider.is_colliding || collider.is_trigger) continue;

        for (auto other : collider.colliding_entities) {
            auto pair = std::minmax(entity, other);
            unique_pairs.insert(pair);
        }
    }

    return unique_pairs;
}

// Utility function to accumulate correction in a map, summing displacements
void OverlapCorrectionSystem::accumulate_correction(
    std::unordered_map<entt::entity, CorrectionIntent>& corrections,
    entt::entity entity,
    const Vector2& offset) {

    corrections[entity].offset.x += offset.x;
    corrections[entity].offset.y += offset.y;
}

// Calculates corrections for all pairs using parallelism for performance
std::unordered_map<entt::entity, CorrectionIntent> OverlapCorrectionSystem::calculate_all_corrections(
    const std::set<std::pair<entt::entity, entt::entity>>& pairs) {

    std::vector pairs_vec(pairs.begin(), pairs.end());

    // Uses transform_reduce to map pairs to corrections and then aggregate them
    return std::transform_reduce(
        std::execution::par,
        pairs_vec.begin(),
        pairs_vec.end(),
        std::unordered_map<entt::entity, CorrectionIntent>{},

        // Merge function to aggregate partial results
        [](auto a, const auto& b) {
            for (auto& [entity, intent] : b) {
                a[entity].offset.x += intent.offset.x;
                a[entity].offset.y += intent.offset.y;
            }
            return a;
        },

        // Function that calculates corrections for a pair
        [&](const auto& pair) {
            std::unordered_map<entt::entity, CorrectionIntent> local;

            auto entityA = pair.first;
            auto entityB = pair.second;

            if (!registry->valid(entityA) || !registry->valid(entityB))
                return local;

            auto& colliderA = registry->get<BoxCollider2D>(entityA);
            auto& colliderB = registry->get<BoxCollider2D>(entityB);
            auto& transformA = registry->get<Transform>(entityA);
            auto& transformB = registry->get<Transform>(entityB);

            const CollisionContext ctx{entityA, entityB, colliderA, colliderB, transformA, transformB};
            const auto overlap = calculate_overlap(ctx);

            if (!overlap.has_penetration())
                return local;

            Vector2 correction = compute_correction(overlap);

            // If any collider is static, apply the offset to the other one
            if (colliderA.is_static || colliderB.is_static) {
                if (colliderA.is_static && !colliderB.is_static) {
                    accumulate_correction(local, entityB, {correction.x * 2.0f, correction.y * 2.0f});
                } else if (!colliderA.is_static && colliderB.is_static) {
                    accumulate_correction(local, entityA, {correction.x * 2.0f, correction.y * 2.0f});
                } else {
                    accumulate_correction(local, entityA, correction);
                    accumulate_correction(local, entityB, {-correction.x, -correction.y});
                }
            } else {
                accumulate_correction(local, entityA, correction);
                accumulate_correction(local, entityB, {-correction.x, -correction.y});
            }

            return local;
        }
    );
}

// Applies corrections to entity transforms only if they exceed epsilon
void OverlapCorrectionSystem::apply_corrections(
    const std::unordered_map<entt::entity, CorrectionIntent>& corrections) {

    for (auto& [entity, intent] : corrections) {
        if (!registry->valid(entity)) continue;

        auto& transform = registry->get<Transform>(entity);
        bool apply_x = std::abs(intent.offset.x) > EPSILON;
        bool apply_y = std::abs(intent.offset.y) > EPSILON;

        if (apply_x || apply_y) {
            transform.position = Vector2Add(transform.position, intent.offset);
        }
    }
}

// Checks if there is still any overlap after applying corrections
bool OverlapCorrectionSystem::check_any_overlap() const {
    auto view = registry->view<BoxCollider2D, Transform>();

    for (auto [entity, collider, transform] : view.each()) {
        if (!collider.is_colliding || collider.is_trigger) continue;

        for (const auto other : collider.colliding_entities) {
            if (!registry->valid(entity) || !registry->valid(other)) continue;

            auto& colliderA = registry->get<BoxCollider2D>(entity);
            auto& colliderB = registry->get<BoxCollider2D>(other);
            auto& transformA = registry->get<Transform>(entity);
            auto& transformB = registry->get<Transform>(other);

            const CollisionContext ctx{entity, other, colliderA, colliderB, transformA, transformB};
            const auto overlap = calculate_overlap(ctx);

            if (overlap.has_penetration()) {
                return true;
            }
        }
    }
    return false;
}

// Calculates the delta vector and overlap values between two colliders
OverlapResult OverlapCorrectionSystem::calculate_overlap(const CollisionContext& ctx) {
    float half_a_w = ctx.collider_a.width * 0.5f;
    float half_a_h = ctx.collider_a.height * 0.5f;
    float half_b_w = ctx.collider_b.width * 0.5f;
    float half_b_h = ctx.collider_b.height * 0.5f;

    Vector2 delta = {
        ctx.transform_a.position.x - ctx.transform_b.position.x,
        ctx.transform_a.position.y - ctx.transform_b.position.y
    };

    float overlap_x = (half_a_w + half_b_w) - std::abs(delta.x);
    float overlap_y = (half_a_h + half_b_h) - std::abs(delta.y);

    return {delta, overlap_x, overlap_y};
}

// Computes the minimal correction vector to resolve the overlap (least penetration)
Vector2 OverlapCorrectionSystem::compute_correction(const OverlapResult& overlap) {
    if (overlap.x <= 0.0f || overlap.y <= 0.0f) {
        return {0.0f, 0.0f};
    }

    if (overlap.x < overlap.y) {
        float sign_x = (overlap.delta.x > 0) ? 1.0f : -1.0f;
        return {overlap.x * sign_x * 0.5f, 0.0f};
    } else {
        float sign_y = (overlap.delta.y > 0) ? 1.0f : -1.0f;
        return {0.0f, overlap.y * sign_y * 0.5f};
    }
}

// Main execution of the system, performs up to MAX_ITERATIONS to resolve all overlaps
void OverlapCorrectionSystem::run(float dt) {
    bool has_overlap = true;
    int iteration = 0;

    while (has_overlap && iteration < MAX_ITERATIONS) {
        // 1. Collect pairs of entities in collision
        auto overlapping_pairs = collect_overlapping_pairs();

        // 2. Calculate corrections for all detected pairs
        auto corrections = calculate_all_corrections(overlapping_pairs);

        // 3. Apply accumulated corrections to the entity transforms
        apply_corrections(corrections);

        // 4. Check if overlaps still exist to decide whether to continue iterating
        has_overlap = check_any_overlap();

        iteration++;
    }
}

} // namespace rpg
