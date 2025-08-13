//
// Created by jhone on 12/08/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include "entt/entity/registry.hpp"
namespace rpg {

class Scene {
protected:
    entt::registry* registry;
public:
    virtual ~Scene() = default;

    explicit Scene(entt::registry* registry) {
        this->registry = registry;
    };
    virtual void init() =0;

};

} // rpg

#endif //SCENE_H
