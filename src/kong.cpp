#include <raylib.h>
#include <raymath.h>
#include "game.h"
#include "constants.h"


/////////////////
// Kong Entity //
/////////////////

const float COOLDOWN = 5;

Kong::Kong(float x, float y) {
    pos = Vector2{x,y};
    size = 2*UNIT;
    animationTime = 0;
    animationState = 0;
}

void Kong::update(void* data, float delta) {
    GameData* d = (GameData*) data;
    animationTime += delta;

    if (animationState == 0) {
        if (animationTime > 5) {
            animationTime = 0;
            animationState = GetRandomValue(1, 3);
        }
    }

    if (animationState == 1 || animationState == 2) {
        if (animationTime > 0.3) {
            animationTime = 0;
            animationState = animationState == 1 ? 2 : 1;
            if (animationState == 1) {
                d->entities.push_back(new KongBarrel(GetRandomValue(-UNIT, 1000), -UNIT, 0));
            }
            if (GetRandomValue(1, 10) == 5) {
                animationState = 0;
            }
        }
    }

    if (animationState == 3) {
        if (animationTime > 2.0) {
            animationTime = 0;
            animationState = 0;
            // Add Barrel
            d->entities.push_back(new KongBarrel(pos.x+size, pos.y-size/2, 1));
        }
    }

}

void Kong::draw() {
    Color c;
    switch (animationState) {
    case 0: c = PURPLE; break; // Still
    case 1: c = PINK; break; // Beating chest frame 1 
    case 2: c = RED; break; // Beating chest frame 2
    case 3: c = BLUE; break; // Throwing
    }
    DrawRectangle(pos.x, pos.y-size/2, size, size, c);
}




/////////////////
// Kong Barrel //
/////////////////

KongBarrel::KongBarrel(float x, float y, int type) {
    size = UNIT;
    pos = Vector2 {x, y};
    this->type = type;
}

void KongBarrel::update(void* data, float delta) {
    GameData* d = (GameData*) data;
    if (type == 0) { // type Rock
        pos.y += 5 * UNIT * delta;
    } else {
        // Caclulate movements for falling barrels
    }
}
bool KongBarrel::isValid() {
    return true;
}

void KongBarrel::draw() {
    DrawRectangle(pos.x, pos.y, size, size, type == 0 ? LIGHTGRAY : BROWN);
}
