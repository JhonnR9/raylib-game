//
// Created by jhone on 12/08/2025.
//

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H
#include "system.h"
#include "entt/entity/registry.hpp"

namespace rpg {
#include "entt/entity/registry.hpp"
class RenderSystem final : public System {

public:
    explicit RenderSystem(entt::registry* registry);
    void run(float dt) override;
};


} // rpg

#endif //RENDER_SYSTEM_H
