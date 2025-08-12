//
// Created by jhone on 12/08/2025.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "raylib.h"

namespace rpg {
    struct Transform {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
    };
}
#endif //TRANSFORM_H
