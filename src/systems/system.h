//
// Created by jhone on 12/08/2025.
//

#ifndef SYSTEM_H
#define SYSTEM_H
#include "entt/entity/registry.hpp"

namespace rpg {
    class System {
    protected:
        entt::registry *registry;

    public:
        explicit System(entt::registry *registry) {
            this->registry = registry;
        };

        virtual ~System() = default;

        virtual void run(float dt) =0;
    };
} // rpg

#endif //SYSTEM_H
