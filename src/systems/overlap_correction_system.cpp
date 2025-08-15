#include "overlap_correction_system.h"
#include "components/components.h"
#include <iostream>

namespace rpg {
    std::size_t OverlapCorrectionSystem::EntityPairHash::operator()(
        const std::pair<entt::entity, entt::entity> &pair) const {
        // Normalize entity order so hash is symmetric
        const entt::entity min_entity = std::min(pair.first, pair.second);
        const entt::entity max_entity = std::max(pair.first, pair.second);

        const size_t hash_min = std::hash<entt::entity>()(min_entity);
        const size_t hash_max = std::hash<entt::entity>()(max_entity);

        // Combine hashes using a variant of boost::hash_combine
        return hash_min ^ (hash_max + 0x9e3779b9 + (hash_min << 6) + (hash_min >> 2));
    }

    bool OverlapCorrectionSystem::EntityPairEqual::operator()(const std::pair<entt::entity, entt::entity> &lhs,
                                                              const std::pair<entt::entity, entt::entity> &rhs) const {
        // Equal if both entities match, regardless of order
        return (lhs.first == rhs.first && lhs.second == rhs.second) ||
               (lhs.first == rhs.second && lhs.second == rhs.first);
    }

    OverlapCorrectionSystem::OverlapCorrectionSystem(entt::registry *registry)
        : System(registry) {
    }

    bool OverlapCorrectionSystem::should_skip_entity(const BoxCollider2D &collider) {
        // Skip if not colliding, marked as trigger, or has no collision contacts
        return !collider.is_colliding || collider.is_trigger || collider.colliding_entities.empty();
    }

    bool OverlapCorrectionSystem::is_pair_already_processed(entt::entity entity_a, entt::entity entity_b) {
        auto pair_key = std::minmax(entity_a, entity_b);
        if (processed_pairs.contains(pair_key)) return true;
        processed_pairs.insert(pair_key);
        return false;
    }

    std::pair<float, float> OverlapCorrectionSystem::compute_overlap(
        const BoxCollider2D &collider_a, const Transform &transform_a,
        const BoxCollider2D &collider_b, const Transform &transform_b) {
        // Compute half-dimensions for easier center-based calculations
        const float half_width_a = collider_a.width * 0.5f;
        const float half_height_a = collider_a.height * 0.5f;
        const float half_width_b = collider_b.width * 0.5f;
        const float half_height_b = collider_b.height * 0.5f;

        // Distance vector between collider centers
        const Vector2 delta = {
            transform_a.position.x - transform_b.position.x,
            transform_a.position.y - transform_b.position.y
        };

        // Positive overlap means penetration depth
        float overlap_x = (half_width_a + half_width_b) - std::abs(delta.x);
        float overlap_y = (half_height_a + half_height_b) - std::abs(delta.y);

        return {overlap_x, overlap_y};
    }

    bool OverlapCorrectionSystem::select_movable_entity(
        const entt::entity entity_a, entt::entity entity_b,
        const BoxCollider2D &collider_a, const BoxCollider2D &collider_b,
        Transform *&movable_transform, MovementData *&movable_movement) const {
        // Prefer non-static entities for movement
        if (!collider_a.is_static) {
            movable_transform = &registry->get<Transform>(entity_a);
            movable_movement = &registry->get<MovementData>(entity_a);
            return true;
        }
        if (!collider_b.is_static && registry->all_of<MovementData>(entity_b)) {
            movable_transform = &registry->get<Transform>(entity_b);
            movable_movement = &registry->get<MovementData>(entity_b);
            return true;
        }
        return false;
    }

    void OverlapCorrectionSystem::apply_overlap_resolution(Vector2 delta, float overlap_x, float overlap_y,
                                                           Transform *transform, MovementData *movement) {
        // Resolve along the axis with less penetration to minimize movement
        if (overlap_x < overlap_y) {
            const float sign_x = (delta.x > 0) ? 1.0f : -1.0f;
            transform->position.x += overlap_x * sign_x;
            movement->velocity.x = 0.0f; // Stop horizontal motion
        } else {
            const float sign_y = (delta.y > 0) ? 1.0f : -1.0f;
            transform->position.y += overlap_y * sign_y;
            movement->velocity.y = 0.0f; // Stop vertical motion
        }
    }

    void OverlapCorrectionSystem::draw_debug_collider(const Transform &transform, const BoxCollider2D &collider) {
        // Simple rectangle outline for visual debugging
        DrawRectangleLines(
            static_cast<int>(transform.position.x - collider.width / 2),
            static_cast<int>(transform.position.y - collider.height / 2),
            static_cast<int>(collider.width),
            static_cast<int>(collider.height),
            PURPLE
        );
    }

    void OverlapCorrectionSystem::run(float delta_time) {
        const auto collidable_view = registry->view<BoxCollider2D, Transform, MovementData>();

        for (auto [entity_id, collider, transform, movement]: collidable_view.each()) {
            if (should_skip_entity(collider)) continue;

            for (auto other_entity_id: collider.colliding_entities) {
                if (is_pair_already_processed(entity_id, other_entity_id)) continue;

                auto &other_collider = registry->get<BoxCollider2D>(other_entity_id);
                auto &other_transform = registry->get<Transform>(other_entity_id);

                auto [overlap_x, overlap_y] = compute_overlap(collider, transform, other_collider, other_transform);
                if (overlap_x <= 0 || overlap_y <= 0) continue; // No penetration

                // Skip if both entities are immovable
                if (collider.is_static && other_collider.is_static) continue;

                Transform *movable_transform = nullptr;
                MovementData *movable_movement = nullptr;

                if (select_movable_entity(entity_id, other_entity_id, collider, other_collider,
                                          movable_transform, movable_movement)) {
                    Vector2 delta = {
                        transform.position.x - other_transform.position.x,
                        transform.position.y - other_transform.position.y
                    };
                    apply_overlap_resolution(delta, overlap_x, overlap_y, movable_transform, movable_movement);
                }
            }

            processed_pairs.clear();
            draw_debug_collider(transform, collider);
        }
    }
} // namespace rpg
