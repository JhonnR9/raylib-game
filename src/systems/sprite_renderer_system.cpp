#include "sprite_renderer_system.h"
#include <fstream>
#include <iostream>
#include "raymath.h"
#include "rlgl.h"
#include "components/components.h"

namespace rpg {
    bool SpriteRendererSystem::load_resources() {
        const Image atlas_image = LoadImage(RESOURCE_PATH "/atlas.png");

        if (!atlas_image.data) {
            std::cerr << "Failed to load " << RESOURCE_PATH "/atlas.png" << std::endl;
            return false;
        }

        atlas_texture = LoadTextureFromImage(atlas_image);
        UnloadImage(atlas_image);

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

    SpriteRendererSystem::SpriteRendererSystem(entt::registry *registry)
        : System(registry) {
        if (!load_resources()) return;

        for (auto &[key, value]: json_data.items()) {
            std::string name = key;

            int x = value["x"];
            int y = value["y"];
            int width = value["width"];
            int height = value["height"];

            Rectangle rect(
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(width),
                static_cast<float>(height)
            );

            uvs[name] = rect;
        }
    }

    void SpriteRendererSystem::run(float dt) {
        auto view = registry->view<Transform, Sprite>();

        if (atlas_texture.id == 0) {
            std::cerr << "Atlas texture not loaded, skipping rendering." << std::endl;
            return;
        }

        rlSetTexture(atlas_texture.id);
        rlBegin(RL_QUADS);

        for (auto [entity, transform, sprite]: view.each()) {
            Rectangle source = uvs[sprite.name];

            bool flipX = false;
            if (source.width < 0) {
                flipX = true;
                source.width *= -1;
            }

            if (source.height < 0) {
                source.y -= source.height;
            }

            Rectangle dest = {
                transform.position.x,
                transform.position.y,
                source.width,
                source.height
            };

            Vector2 origin = {
                sprite.size.x * 0.5f,
                sprite.size.y * 0.5f
            };

            // Cálculo dos vértices com rotação
            Vector2 verts[4];
            if (transform.rotation == 0.0f) {
                float x = dest.x - origin.x;
                float y = dest.y - origin.y;

                verts[0] = {x, y}; // Top-left
                verts[1] = {x, y + dest.height}; // Bottom-left
                verts[2] = {x + dest.width, y + dest.height}; // Bottom-right
                verts[3] = {x + dest.width, y}; // Top-right
            } else {
                float sinRot = sinf(transform.rotation * DEG2RAD);
                float cosRot = cosf(transform.rotation * DEG2RAD);
                float x = dest.x;
                float y = dest.y;
                float dx = -origin.x;
                float dy = -origin.y;

                verts[0] = {x + dx * cosRot - dy * sinRot, y + dx * sinRot + dy * cosRot}; // TL
                verts[1] = {
                    x + dx * cosRot - (dy + dest.height) * sinRot,
                    y + dx * sinRot + (dy + dest.height) * cosRot
                }; // BL
                verts[2] = {
                    x + (dx + dest.width) * cosRot - (dy + dest.height) * sinRot,
                    y + (dx + dest.width) * sinRot + (dy + dest.height) * cosRot
                }; // BR
                verts[3] = {
                    x + (dx + dest.width) * cosRot - dy * sinRot,
                    y + (dx + dest.width) * sinRot + dy * cosRot
                }; // TR
            }

            float texWidth = (float) atlas_texture.width;
            float texHeight = (float) atlas_texture.height;

            float sx = source.x / texWidth;
            float sy = source.y / texHeight;
            float sw = source.width / texWidth;
            float sh = source.height / texHeight;

            float tx[4] = {
                flipX ? sx + sw : sx,
                flipX ? sx + sw : sx,
                flipX ? sx : sx + sw,
                flipX ? sx : sx + sw
            };

            float ty[4] = {
                sy,
                sy + sh,
                sy + sh,
                sy
            };

            rlColor4ub(sprite.color.r, sprite.color.g, sprite.color.b, sprite.color.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);

            // Enviar vértices + UVs
            for (int i = 0; i < 4; ++i) {
                rlTexCoord2f(tx[i], ty[i]);
                rlVertex2f(verts[i].x, verts[i].y);
            }
        }

        rlEnd();
        rlSetTexture(0);
    }
} // namespace rpg
