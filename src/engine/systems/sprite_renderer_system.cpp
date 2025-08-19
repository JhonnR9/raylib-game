// SpriteRendererSystem handles 2D sprite rendering using a texture atlas.
// It uses data from the Transform and Sprite components (via entt ECS).
// Rendering is done via rlgl (low-level API), so it's fully manual:
// UVs, rotation, and vertex submission are computed and pushed per frame.


#include "sprite_renderer_system.h"
#include <fstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "rlgl.h"
#include "engine/components/components.h"

namespace rpg {

    // Loads texture atlas image and JSON metadata.
    // This must succeed for the renderer to function correctly.
    bool SpriteRendererSystem::load_resources() {
        const Image atlas_image = LoadImage(RESOURCE_PATH"/atlas.png");

        if (!atlas_image.data) {
            std::cerr << "Failed to load " << RESOURCE_PATH "/atlas.png" << std::endl;
            return false;
        }

        // Upload the atlas to GPU and unload the CPU-side image.
        atlas_texture = LoadTextureFromImage(atlas_image);
        UnloadImage(atlas_image);

        // Open JSON file with sprite UV coordinates and dimensions.
        std::ifstream json_file(RESOURCE_PATH "/atlas.json");
        if (!json_file.is_open()) {
            std::cerr << "Failed to open " << RESOURCE_PATH "/atlas.json" << std::endl;
            return false;
        }

        json_data = nlohmann::json::parse(json_file);
        if (!json_data.is_object()) {
            std::cerr << "Failed to parse atlas.json" << std::endl;
            return false;
        }

        return true;
    }

    // Calculates the four corners of a quad, applying 2D rotation around a pivot (origin).
    // Results are stored in `verts_cache` for later GPU upload.
    void SpriteRendererSystem::apply_rotation(
        const Rectangle &dest,
        const Vector2 &origin,
        const float rotation_deg
    ) {
        if (rotation_deg == 0.0f) {
            // No rotation: just offset by the origin.
            const float x = dest.x - origin.x;
            const float y = dest.y - origin.y;

            verts_cache[0] = {x, y}; // Top-left
            verts_cache[1] = {x, y + dest.height}; // Bottom-left
            verts_cache[2] = {x + dest.width, y + dest.height}; // Bottom-right
            verts_cache[3] = {x + dest.width, y}; // Top-right
        } else {
            // Apply rotation using basic 2D rotation matrix
            const float sinRot = sinf(rotation_deg * DEG2RAD);
            const float cosRot = cosf(rotation_deg * DEG2RAD);
            const float x = dest.x;
            const float y = dest.y;
            const float dx = -origin.x;
            const float dy = -origin.y;

            verts_cache[0] = {x + dx * cosRot - dy * sinRot, y + dx * sinRot + dy * cosRot}; // TL
            verts_cache[1] = {
                x + dx * cosRot - (dy + dest.height) * sinRot, y + dx * sinRot + (dy + dest.height) * cosRot
            }; // BL
            verts_cache[2] = {
                x + (dx + dest.width) * cosRot - (dy + dest.height) * sinRot,
                y + (dx + dest.width) * sinRot + (dy + dest.height) * cosRot
            }; // BR
            verts_cache[3] = {
                x + (dx + dest.width) * cosRot - dy * sinRot,
                y + (dx + dest.width) * sinRot + dy * cosRot
            }; // TR
        }
    }

    // Constructor: Initializes the system and loads sprite atlas metadata into memory.
    SpriteRendererSystem::SpriteRendererSystem(entt::registry *registry)
        : System(registry) {
        if (!load_resources()) return;

        // Iterate over each sprite entry in the atlas.json
        for (auto &[key, value]: json_data.items()) {
            std::string name = key;

            const float x = value["x"];
            const float y = value["y"];
            const float width = (value["width"]);
            const float height = (value["height"]);

            const auto tex_width = static_cast<float>(atlas_texture.width);
            const auto tex_height = static_cast<float>(atlas_texture.height);

            // Calculate normalized UV texture coordinates (0.0 to 1.0)
            const float sx = x / tex_width;
            const float sy = y / tex_height;
            const float sw = width / tex_width;
            const float sh = height / tex_height;

            // Store sprite metadata (dimensions + UVs) for fast access during rendering
            normalized_uvs[name] = {width, height, sx, sy, sw, sh};
        }
    }

    // Main rendering function. Called every frame to draw all entities with Transform + Sprite.
    void SpriteRendererSystem::run(float dt) {
        const auto view = registry->view<rpg::Transform, Sprite>();

        if (atlas_texture.id == 0) {
            std::cerr << "Atlas texture not loaded, skipping rendering." << std::endl;
            return;
        }

        // Begin drawing textured quads using low-level rlgl API
        rlSetTexture(atlas_texture.id);
        rlBegin(RL_QUADS);

        for (auto [entity, transform, sprite]: view.each()) {


            auto& [width, height, sx, sy, sw, sh] = normalized_uvs[sprite.name];

            bool flipX = false;

            // Flip horizontally if width is negative
            if (width < 0) {
                flipX = true;
                width *= -1;
            }

            // Adjust Y UV coordinate for vertically flipped sprites
            if (height < 0) {
                sy -= height;
            }

            // Destination rectangle on screen
            const Rectangle dest = {
                transform.position.x,
                transform.position.y,
                width,
                height
            };

            // Pivot point for rotation (centered on sprite)
            const Vector2 origin = {
                sprite.size.x * 0.5f,
                sprite.size.y * 0.5f
            };

            // Compute final vertex positions with rotation
            apply_rotation(dest, origin, transform.rotation);

            // UV coordinates per corner (flipped if needed)
            const float tx[4] = {
                flipX ? sx + sw : sx,
                flipX ? sx + sw : sx,
                flipX ? sx : sx + sw,
                flipX ? sx : sx + sw
            };

            const float ty[4] = {
                sy,
                sy + sh,
                sy + sh,
                sy
            };

            // Set color and normal
            rlColor4ub(sprite.color.r, sprite.color.g, sprite.color.b, sprite.color.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);

            // Submit 4 vertices (1 quad) to the GPU
            for (int i = 0; i < 4; ++i) {
                rlTexCoord2f(tx[i], ty[i]);
                rlVertex2f(verts_cache[i].x, verts_cache[i].y);
            }
        }

        rlEnd();
        rlSetTexture(0); // Unbind texture
    }

} // namespace rpg
