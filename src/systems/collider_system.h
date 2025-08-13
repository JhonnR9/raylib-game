//
// Created by jhone on 13/08/2025.
//

#ifndef COLLIDER_SYSTEM_H
#define COLLIDER_SYSTEM_H
#include "system.h"

namespace rpg {

class ColliderSystem: public System{

    public:
    explicit ColliderSystem(entt::registry* registry);
    void run(float dt) override;

};

} // rpg

#endif //COLLIDER_SYSTEM_H
