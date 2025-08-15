//
// Created by jhone on 14/08/2025.
//

#ifndef OVERLAP_CORRECTION_SYSTEM_H
#define OVERLAP_CORRECTION_SYSTEM_H
#include "system.h"
#include <unordered_set>

namespace rpg {

class OverlapCorrectionSystem final : public System{
    struct UnorderedEntityHash {
        std::size_t operator()(const std::pair<entt::entity, entt::entity> &p) const {
            entt::entity a = std::min(p.first, p.second);
            entt::entity b = std::max(p.first, p.second);
            size_t h1 = std::hash<entt::entity>()(a);
            size_t h2 = std::hash<entt::entity>()(b);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    struct UnorderedEntityEqual {
        bool operator()(const std::pair<entt::entity, entt::entity> &lhs,
                        const std::pair<entt::entity, entt::entity> &rhs) const {
            return (lhs.first == rhs.first && lhs.second == rhs.second) ||
                   (lhs.first == rhs.second && lhs.second == rhs.first);
        }
    };

    std::unordered_set<std::pair<entt::entity, entt::entity>, UnorderedEntityHash, UnorderedEntityEqual> pairs_checked;

public:
    explicit  OverlapCorrectionSystem(entt::registry* registry);
    void run(float dt) override;
};

} // rpg

#endif //OVERLAP_CORRECTION_SYSTEM_H
