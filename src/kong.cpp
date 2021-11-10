#include <raylib.h>
#include <raymath.h>
#include "game.h"
#include "constants.h"


/////////////////
// Kong Entity //
/////////////////

const float COOLDOWN = 5;
const float BARREL_SPEED = 3 * UNIT;

Kong::Kong(float x, float y) {
    pos = Vector2{x,y};
    size = 2*UNIT;
    animationTime = 0;
    animationState = 0;
}

void Kong::update(GameData* d, float delta) {
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
    if (type == 0) {
        vel = Vector2 {0, 2 * BARREL_SPEED};
    } else {
        vel = Vector2 {BARREL_SPEED, 0};
    }
    this->type = type;
    preferLeft = false;
    bounces = 0;
}

void KongBarrel::update(GameData* d, float delta) {
    if (type == 0) { // type Rock
        // No update needed
    } else {
        // Calculate movements for falling barrels
        int x = (int)(pos.x / UNIT);
        int y = (int)(pos.y / UNIT);
        auto i = [&d] (int x, int y) { return x + y * d->map.width; };
        auto brec = Rectangle{pos.x, pos.y, size, size};
        auto collides = [&i, &d, &brec](int x, int y) {
            int t = d->map.collisiondata[i(x, y)];
            if (t == COL_SOLID || t == COL_ONE_WAY) {
                auto trec = Rectangle{x * UNIT, y * UNIT, UNIT, UNIT};
                return CheckCollisionRecs(trec, brec);
            }
            return false;
        };
        bool bottom = collides(x-1, y+1) || collides(x, y+1) || collides(x+1, y+1);
        bool left = collides(x, y);
        bool right = collides(x+1, y);

        if (!bottom) { // Nothing underneath
            vel = Vector2 {0, BARREL_SPEED};
        } else if (!preferLeft && !right) { // Nothing right
            vel = Vector2 {BARREL_SPEED, 0};
        } else if (!preferLeft && right) {
            preferLeft = true;
            vel = Vector2 {-BARREL_SPEED, 0};
            bounces ++;
        } else if (preferLeft && !left) { // Nothing Left
            vel = Vector2 {-BARREL_SPEED, 0};
        } else if (preferLeft && left) {
            preferLeft = false;
            vel = Vector2 {BARREL_SPEED, 0};
            bounces ++;
        } else {
        }
    }
    pos.x += vel.x * delta;
    pos.y += vel.y * delta;
}
bool KongBarrel::isValid() {
    return bounces <= 3;
}

void KongBarrel::draw() {
    DrawRectangle(pos.x, pos.y, size, size, type == 0 ? LIGHTGRAY : BROWN);
}
