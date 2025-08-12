//
// Created by jhone on 12/08/2025.
//

#ifndef BOXCOLLIDER2D_H
#define BOXCOLLIDER2D_H

namespace rpg {
    struct BoxCollider2D {
        float width = 0;
        float height = 0;
        bool collided = false;
        bool isTrigger = false;
    };
} // rpg

#endif //BOXCOLLIDER2D_H
