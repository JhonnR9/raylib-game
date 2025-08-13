//
// Created by jhone on 12/08/2025.
//

#ifndef APP_H
#define APP_H
#include <memory>
#include <vector>

#include "scenes/scene.h"
#include "systems/system.h"

namespace rpg {
    class APP {
        std::unique_ptr<Scene> scene;
        std::vector<std::unique_ptr<System>> systems;
        std::unique_ptr<entt::registry> registry;

    public:
        APP();

        ~APP();

        void run() const;
    };
} // rpg

#endif //APP_H
