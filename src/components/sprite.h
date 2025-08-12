//
// Created by jhone on 12/08/2025.
//

#ifndef SPRITE_H
#define SPRITE_H
#include "raylib.h"

namespace rpg {

struct  Sprite {
    Texture2D* texture;
    Vector2 uv;
    Vector2 size;
};

} // rpg

#endif //SPRITE_H
