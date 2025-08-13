//
// Created by jhone on 13/08/2025.
//

#ifndef MOVE_SYSTEM_H
#define MOVE_SYSTEM_H
#include "system.h"
#include "entt/entity/registry.hpp"

namespace rpg {

class MoveSystem : public System{
public:
    explicit MoveSystem(entt::registry* registry);
    void run(float dt) override;
};

} // rpg

#endif //MOVE_SYSTEM_H
