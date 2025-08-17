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

class SpriteRendererSystem final : public System {
    struct SpriteUV {
        float width, height;
        float sx, sy, sw, sh;
    };
    Texture2D atlas_texture{};
    nlohmann::json json_data;

    std::unordered_map<std::string, SpriteUV> normalized_uvs;
    bool load_resources();
    std::array<Vector2, 4>  verts_cache{};

    void apply_rotation(const Rectangle &dest, const Vector2 &origin, float rotation_deg);
public:
    explicit SpriteRendererSystem(entt::registry *registry);
    void run(float dt) override;
};

} // rpg

#endif //SPRITE_RENDERER_SYSTEM_H
