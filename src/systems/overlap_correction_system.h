//
// Created by jhone on 14/08/2025.
//

#ifndef OVERLAP_CORRECTION_SYSTEM_H
#define OVERLAP_CORRECTION_SYSTEM_H
#include "system.h"
namespace rpg {

class OverlapCorrectionSystem final : public System{
public:
    explicit  OverlapCorrectionSystem(entt::registry* registry);
    void run(float dt) override;
};

} // rpg

#endif //OVERLAP_CORRECTION_SYSTEM_H
