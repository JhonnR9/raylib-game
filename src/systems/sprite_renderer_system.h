//
// Created by jhone on 17/08/2025.
//

#ifndef SPRITE_RENDERER_SYSTEM_H
#define SPRITE_RENDERER_SYSTEM_H
#include "raylib.h"
#include "system.h"
#include  <nlohmann/json.hpp>
#include <unordered_map>

namespace rpg {

class SpriteRendererSystem : public System {
    Texture2D atlas_texture{};
    nlohmann::json json_data;

    std::unordered_map<std::string, Rectangle> uvs;
    bool load_resources();
public:
    explicit SpriteRendererSystem(entt::registry *registry);
    void run(float dt) override;
};

} // rpg

#endif //SPRITE_RENDERER_SYSTEM_H
