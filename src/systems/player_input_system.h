//
// Created by jhone on 13/08/2025.
//

#ifndef PLAYER_INPUT_SYSTEM_H
#define PLAYER_INPUT_SYSTEM_H
#include "system.h"

namespace rpg {

class PlayerInputSystem : public System {
public:
    explicit PlayerInputSystem(entt::registry* registry);
    void run(float dt) override;

};

} // rpg

#endif //PLAYER_INPUT_SYSTEM_H
