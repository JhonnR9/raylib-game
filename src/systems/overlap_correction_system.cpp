#include "overlap_correction_system.h"
#include "components/components.h"
#include <iostream>

namespace rpg {
    // ---------------------- Hash & Equal ----------------------

    std::size_t OverlapCorrectionSystem::EntityPairHash::operator()(
        const std::pair<entt::entity, entt::entity> &pair) const {
        auto [min_entity, max_entity] = std::minmax(pair.first, pair.second);
        size_t hash_min = std::hash<entt::entity>()(min_entity);
        size_t hash_max = std::hash<entt::entity>()(max_entity);
        return hash_min ^ (hash_max + 0x9e3779b9 + (hash_min << 6) + (hash_min >> 2));
    }

    bool OverlapCorrectionSystem::EntityPairEqual::operator()(
        const std::pair<entt::entity, entt::entity> &lhs,
        const std::pair<entt::entity, entt::entity> &rhs) const {
        return (lhs.first == rhs.first && lhs.second == rhs.second) ||
               (lhs.first == rhs.second && lhs.second == rhs.first);
    }

    // ---------------------- Constructor ----------------------

    OverlapCorrectionSystem::OverlapCorrectionSystem(entt::registry *registry)
        : System(registry) {
    }

    // ---------------------- Helpers ----------------------

    bool OverlapCorrectionSystem::should_skip_entity(const BoxCollider2D &collider) {
        return !collider.is_colliding || collider.is_trigger || collider.colliding_entities.empty();
    }

    bool OverlapCorrectionSystem::has_already_processed(entt::entity a, entt::entity b) {
        auto pair_key = std::minmax(a, b);
        if (processed_pairs.contains(pair_key)) return true;
        processed_pairs.insert(pair_key);
        return false;
    }

    OverlapResult OverlapCorrectionSystem::calculate_overlap(const CollisionContext &ctx) {
        float half_a_w = ctx.colliderA.width * 0.5f;
        float half_a_h = ctx.colliderA.height * 0.5f;
        float half_b_w = ctx.colliderB.width * 0.5f;
        float half_b_h = ctx.colliderB.height * 0.5f;

        Vector2 delta = {
            ctx.transformA.position.x - ctx.transformB.position.x,
            ctx.transformA.position.y - ctx.transformB.position.y
        };

        float overlap_x = (half_a_w + half_b_w) - std::abs(delta.x);
        float overlap_y = (half_a_h + half_b_h) - std::abs(delta.y);

        return {delta, overlap_x, overlap_y};
    }

    void OverlapCorrectionSystem::resolve_overlap(const OverlapResult &overlap,
                                                  Transform &transform,
                                                  MovementData &movement) {
        if (overlap.x < overlap.y) {
            float sign_x = (overlap.delta.x > 0) ? 1.0f : -1.0f;
            transform.position.x += overlap.x * sign_x;
            movement.velocity.x = 0.0f;
        } else {
            float sign_y = (overlap.delta.y > 0) ? 1.0f : -1.0f;
            transform.position.y += overlap.y * sign_y;
            movement.velocity.y = 0.0f;
        }
    }

    std::optional<std::pair<Transform &, MovementData &> >
    OverlapCorrectionSystem::choose_movable_entity(const CollisionContext &ctx) const {
        if (!ctx.colliderA.is_static && registry->all_of<MovementData>(ctx.entityA)) {
            return std::pair<Transform &, MovementData &>{
                registry->get<Transform>(ctx.entityA),
                registry->get<MovementData>(ctx.entityA)
            };
        }
        if (!ctx.colliderB.is_static && registry->all_of<MovementData>(ctx.entityB)) {
            return std::pair<Transform &, MovementData &>{
                registry->get<Transform>(ctx.entityB),
                registry->get<MovementData>(ctx.entityB)
            };
        }
        return std::nullopt;
    }

    void OverlapCorrectionSystem::resolve_overlap_between(
        const OverlapResult &overlap,
        Transform &transformA, MovementData &movementA,
        Transform &transformB, MovementData &movementB) {
        if (overlap.x < overlap.y) {
            float sign_x = (overlap.delta.x > 0) ? 1.0f : -1.0f;
            float correction = overlap.x * 0.5f;
            transformA.position.x += correction * sign_x;
            transformB.position.x -= correction * sign_x;
            movementA.velocity.x = 0.0f;
            movementB.velocity.x = 0.0f;
        } else {
            float sign_y = (overlap.delta.y > 0) ? 1.0f : -1.0f;
            float correction = overlap.y * 0.5f;
            transformA.position.y += correction * sign_y;
            transformB.position.y -= correction * sign_y;
            movementA.velocity.y = 0.0f;
            movementB.velocity.y = 0.0f;
        }
    }


    void OverlapCorrectionSystem::draw_debug(const CollisionContext &ctx) {
        DrawRectangleLines(
            static_cast<int>(ctx.transformA.position.x - ctx.colliderA.width / 2),
            static_cast<int>(ctx.transformA.position.y - ctx.colliderA.height / 2),
            static_cast<int>(ctx.colliderA.width),
            static_cast<int>(ctx.colliderA.height),
            PURPLE
        );
    }

    // ---------------------- Run ----------------------

    void OverlapCorrectionSystem::run(float delta_time) {
        auto view = registry->view<BoxCollider2D, Transform, MovementData>();

        for (auto [entity, collider, transform, movement]: view.each()) {
            if (should_skip_entity(collider)) continue;

            for (auto other_entity: collider.colliding_entities) {
                if (has_already_processed(entity, other_entity)) continue;

                auto &other_collider = registry->get<BoxCollider2D>(other_entity);
                auto &other_transform = registry->get<Transform>(other_entity);

                CollisionContext ctx{entity, other_entity, collider, other_collider, transform, other_transform};
                auto overlap = calculate_overlap(ctx);

                if (!overlap.has_penetration()) continue;
                if (collider.is_static && other_collider.is_static) continue;

                if (auto movable = choose_movable_entity(ctx)) {
                    auto &[movable_transform, movable_movement] = *movable;
                    resolve_overlap(overlap, movable_transform, movable_movement);
                }
            }

            // draw_debug({entity, entt::null, collider, collider, transform, transform});
        }
        processed_pairs.clear();
    }
} // namespace rpg
