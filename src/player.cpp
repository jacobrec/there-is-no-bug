#include <raylib.h>
#include <raymath.h>
#include "game.h"
#include "constants.h"

Player::Player(float x, float y) {
    pos = Vector2{x, y};
    vel = Vector2{0, 0};
    size = UNIT;
    state = PlayerState::Standing;
    lastJumped = GetTime();
    lastWalljumped = 0;
}

void Player::draw() {
    Color c = BLUE;
    if (this->state == PlayerState::Running) {c = YELLOW;}
    if (this->state == PlayerState::Air) {c = ORANGE;}
    if (this->state == PlayerState::Sliding) {c = RED;}
    if (this->state == PlayerState::Standing) {c = BEIGE;}
    DrawRectangle(this->pos.x, this->pos.y, UNIT, UNIT, c);
}

void updatePlayer(Player* p, GameData *d, float delta) {
    auto effective_max_velocity = d->keys.b ? SPRINT_SPEED_MODIFIER * MAX_VELOCITY : MAX_VELOCITY;
    auto effective_accel = d->keys.b ? SPRINT_ACCEL_MODIFIER * ACCEL : ACCEL;
    if (p->state == PlayerState::Climbing) {
    } else {
        p->state = PlayerState::Standing;
        if (d->keys.left && !d->keys.right) {
            p->vel.x -= effective_accel * delta;
            p->vel.x = max(p->vel.x, -effective_max_velocity);
            p->state = PlayerState::Running;
        } else if (d->keys.right && !d->keys.left) {
            p->vel.x += effective_accel * delta;
            p->vel.x = min(p->vel.x, effective_max_velocity);
            p->state = PlayerState::Running;
        }
    }

    p->vel.y += GRAVITY * delta;

    float px = p->pos.x;
    float py = p->pos.y;
    float ps = p->size;
    float inset = UNIT / 5;
    float colmask = UNIT / 10;
    float tiny_colmask = 1;
    bool grounded = false;
    bool climbing = false;
    int walled = 0;
    for (int yt = (int) (py/UNIT) - 1; yt <= (int) (py/UNIT) + 1; yt++) {
        for (int xt = (int) (px/UNIT) - 1; xt <= (int) (px/UNIT) + 1; xt++) {
            int idx = xt + yt * d->map.width;
            if (yt < d->map.height && yt >=0 && xt < d->map.width && xt >= 0) {
                auto tile = Rectangle{xt*UNIT, yt*UNIT, UNIT, UNIT};
                if (d->map.collisiondata[idx] == COL_CLIMB) {
                    auto pr = Rectangle{px, py, ps, ps};
                    if (p->state == PlayerState::Climbing) {
                        climbing = CheckCollisionRecs(pr, tile);
                    } else {
                        if (CheckCollisionRecs(pr, tile)) {
                            if (d->keys.up || d->keys.down)
                            climbing = true;
                        }
                    }
                } else if (d->map.collisiondata[idx] == COL_ONE_WAY) {
                    auto pb = Rectangle{px+inset/2, py + ps - tiny_colmask, ps-inset, tiny_colmask};
                    auto onewaytile = Rectangle{xt*UNIT, yt*UNIT, UNIT, UNIT/5};
                    if (p->vel.y > 0 && CheckCollisionRecs(pb, onewaytile)) { // Collision Bottom
                        p->pos.y = yt * UNIT - ps + 1;
                        p->vel.y = 0;
                        grounded = true;
                    }
                } else if (d->map.collisiondata[idx] == COL_SOLID) {
                    auto pb = Rectangle{px+inset/2, py + ps - colmask, ps-inset, colmask};
                    auto pt = Rectangle{px+inset/2, py, ps-inset, colmask};
                    auto pl = Rectangle{px, py+inset/2, colmask, ps-inset};
                    auto pr = Rectangle{px+ps-colmask, py+inset/2, colmask, ps-inset};
                    if (CheckCollisionRecs(pb, tile)) { // Collision Bottom
                        p->pos.y = yt * UNIT - ps + 1;
                        p->vel.y = 0;
                        grounded = true;
                    } else if (CheckCollisionRecs(pt, tile)) { // Collision Top
                        p->pos.y = yt * UNIT + UNIT;
                        p->vel.y = 0;
                    } else if (CheckCollisionRecs(pl, tile)) { // Collision Left
                        p->pos.x = xt * UNIT + UNIT - 1;
                        p->vel.x = max(0.0f, p->vel.x);
                        walled = -1;
                    } else if (CheckCollisionRecs(pr, tile)) { // Collision Right
                        p->pos.x = xt * UNIT - ps + 1;
                        p->vel.x = min(0.0f, p->vel.x);
                        walled = 1;
                    }

                }
            }
        }
    }
    if (p->state != PlayerState::Climbing) {
        if (!grounded) {
            if (walled != 0) {
                p->state = PlayerState::Sliding;
            } else {
                p->state = PlayerState::Air;
            }
        }
    }
    if (climbing) {
        p->state = PlayerState::Climbing;
    }
    if (p->state == PlayerState::Climbing && (!climbing || d->keys.b)) {
        p->state = PlayerState::Air;
    }

    if (grounded) {
        p->lastWalljumped = 0;
    }

    bool jumped = false;
    if (d->keys.a) {
        if (p->state == PlayerState::Climbing) {
            p->vel.y = JUMP_VELOCITY;
            jumped = true;
            p->lastJumped = GetTime();
            p->state = PlayerState::Air;
        } else if (grounded && GetTime() - p->lastJumped > JUMP_COOLDOWN) {
            p->vel.y = JUMP_VELOCITY;
            jumped = true;
            p->lastJumped = GetTime();
        } else if (GetTime() - p->lastJumped < JUMP_EXTENSION_TIME) {
            p->vel.y = JUMP_VELOCITY;
        } else if (walled && GetTime() - p->lastJumped > JUMP_COOLDOWN) {
            if (p->lastWalljumped != walled) {
                p->vel.y = JUMP_VELOCITY;
                p->vel.x = - walled * WALLJUMP_VELOCITY;
                jumped = true;
                p->lastJumped = GetTime();
                p->lastWalljumped = walled;
            }
        }
    }

    if (p->state == PlayerState::Standing) {
        p->vel = Vector2Scale(p->vel, FRICTION_FACTOR);
    } else if (p->state == PlayerState::Air) {
        p->vel.x = p->vel.x * AIR_FRICTION_FACTOR;
    } else if (p->state == PlayerState::Climbing) {
        p->vel.x = 0;
        p->vel.y = 0;
        if (d->keys.left) p->vel.x -= CLIMB_SPEED;
        if (d->keys.right) p->vel.x += CLIMB_SPEED;
        if (d->keys.up) p->vel.y -= CLIMB_SPEED;
        if (d->keys.down) p->vel.y += CLIMB_SPEED;
    }

    // printf("Pos (%f,%f). Vel(%f,%f) Delta(%f, %f)\n", p->pos.x, d->player.pos.y, d->player.vel.x, d->player.vel.y, d->player.vel.x * delta, d->player.vel.y * delta);
    auto dv = Vector2Scale(p->vel, delta);
    float eps = 0.1;
    if ((d->keys.right || jumped) && dv.x > eps && dv.x < 1) { dv.x = 1; }
    if ((d->keys.left || jumped) && dv.x < -eps && dv.x > -1) { dv.x = -1; }
    if (jumped && dv.y < -eps && dv.y > -1) { dv.y = -1; }
    p->pos = Vector2Add(p->pos, dv);
    d->cam.target = p->pos;
}

void Player::update(GameData *d, float delta) {
    updatePlayer(this, d, delta);
}


void Player::collidesWith(GameData *data, Entity* other) {
    if (KongBarrel* p = dynamic_cast<KongBarrel*>(other)) {
        data->state = GameState::Failed;
    } else if (WinCondition* p = dynamic_cast<WinCondition*>(other)) {
        data->state = GameState::Succeeded;
    }
}


WinCondition::WinCondition(float x, float y) {
    pos = Vector2{x, y};
    size = UNIT;
}

void WinCondition::draw() {
    DrawRectangle(this->pos.x, this->pos.y, UNIT, UNIT, ColorAlpha(WHITE, 0.5));
}
