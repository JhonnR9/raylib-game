//
// Created by jhone on 17/08/2025.
//

#include "sprite_renderer_system.h"
#include <fstream>
#include <iostream>

#include "raymath.h"
#include "rlgl.h"
#include "components/components.h"

namespace rpg {
    void SpriteRendererSystem::draw(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation,
                                    Color tint) {
        // Check if texture is valid
        if (texture.id > 0) {
            float width = (float) texture.width;
            float height = (float) texture.height;

            bool flipX = false;

            if (source.width < 0) {
                flipX = true;
                source.width *= -1;
            }
            if (source.height < 0) source.y -= source.height;

            if (dest.width < 0) dest.width *= -1;
            if (dest.height < 0) dest.height *= -1;

            Vector2 topLeft = {0};
            Vector2 topRight = {0};
            Vector2 bottomLeft = {0};
            Vector2 bottomRight = {0};

            // Only calculate rotation if needed
            if (rotation == 0.0f) {
                float x = dest.x - origin.x;
                float y = dest.y - origin.y;
                topLeft = (Vector2){x, y};
                topRight = (Vector2){x + dest.width, y};
                bottomLeft = (Vector2){x, y + dest.height};
                bottomRight = (Vector2){x + dest.width, y + dest.height};
            } else {
                float sinRotation = sinf(rotation * DEG2RAD);
                float cosRotation = cosf(rotation * DEG2RAD);
                float x = dest.x;
                float y = dest.y;
                float dx = -origin.x;
                float dy = -origin.y;

                topLeft.x = x + dx * cosRotation - dy * sinRotation;
                topLeft.y = y + dx * sinRotation + dy * cosRotation;

                topRight.x = x + (dx + dest.width) * cosRotation - dy * sinRotation;
                topRight.y = y + (dx + dest.width) * sinRotation + dy * cosRotation;

                bottomLeft.x = x + dx * cosRotation - (dy + dest.height) * sinRotation;
                bottomLeft.y = y + dx * sinRotation + (dy + dest.height) * cosRotation;

                bottomRight.x = x + (dx + dest.width) * cosRotation - (dy + dest.height) * sinRotation;
                bottomRight.y = y + (dx + dest.width) * sinRotation + (dy + dest.height) * cosRotation;
            }

            rlSetTexture(texture.id);
            rlBegin(RL_QUADS);

            rlColor4ub(tint.r, tint.g, tint.b, tint.a);
            rlNormal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards viewer

            // Top-left corner for texture and quad
            if (flipX) rlTexCoord2f((source.x + source.width) / width, source.y / height);
            else rlTexCoord2f(source.x / width, source.y / height);
            rlVertex2f(topLeft.x, topLeft.y);

            // Bottom-left corner for texture and quad
            if (flipX) rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
            else rlTexCoord2f(source.x / width, (source.y + source.height) / height);
            rlVertex2f(bottomLeft.x, bottomLeft.y);

            // Bottom-right corner for texture and quad
            if (flipX) rlTexCoord2f(source.x / width, (source.y + source.height) / height);
            else rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
            rlVertex2f(bottomRight.x, bottomRight.y);

            // Top-right corner for texture and quad
            if (flipX) rlTexCoord2f(source.x / width, source.y / height);
            else rlTexCoord2f((source.x + source.width) / width, source.y / height);
            rlVertex2f(topRight.x, topRight.y);

            rlEnd();
            rlSetTexture(0);
        }
    }

    bool SpriteRendererSystem::load_resources() {
        const Image atlas_image = LoadImage(RESOURCE_PATH"/atlas.png");
        if (!atlas_image.data) {
            std::cerr << "Failed to load " << RESOURCE_PATH"/atlas.png" << std::endl;
            return false;
        }

        atlas_texture = LoadTextureFromImage(atlas_image);

        UnloadImage(atlas_image);

        std::ifstream json_file(RESOURCE_PATH"/atlas.json");

        if (!json_file.is_open()) {
            std::cerr << "Failed to open " << RESOURCE_PATH"/atlas.json" << std::endl;
            return false;
        }

        json_data = nlohmann::json::parse(json_file);

        if (!json_data.is_object()) {
            std::cerr << "Failed to parse atlas.json" << std::endl;
            return false;
        }

        return true;
    }

    SpriteRendererSystem::SpriteRendererSystem(entt::registry *registry): System(registry) {
        if (load_resources()) {
            for (auto &[key, value]: json_data.items()) {
                std::string name = key;
                const int x = value["x"];
                const int y = value["y"];
                const int width = value["width"];
                const int height = value["height"];

                const Rectangle rect(
                    static_cast<float>(x),
                    static_cast<float>(y),
                    static_cast<float>(width),
                    static_cast<float>(height));
                uvs[name] = rect;
            }
        }
    }

    void SpriteRendererSystem::run(float dt) {
        auto view = registry->view<Transform, Sprite>();

        if (atlas_texture.id == 0) {
            std::cerr << "Atlas texture not loaded, skipping rendering." << std::endl;
            return;
        }

        for (auto [entity, transform, sprite]: view.each()) {
            Texture2D texture = atlas_texture;
            Rectangle source;
            Rectangle dest;
            Vector2 origin;
            float rotation;
            Color color;

            source = uvs[sprite.name];
            dest = Rectangle(transform.position.x, transform.position.y, source.width, source.height);
            rotation = transform.rotation;
            origin.x = sprite.size.x * .5f;
            origin.y = sprite.size.y * .5f;
            color = sprite.color;

            draw(texture, source, dest, origin, rotation, color);
        }
    }
} // rpg
