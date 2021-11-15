#include <functional>
#include <raylib.h>
#include <raymath.h>
#include "game.h"
#include "util.h"
#include "constants.h"
#include "extras/toml.hpp"


Animation::Animation(string aniPath) {
    toml::table tbl;
    try {
        tbl = toml::parse_file(aniPath + "/animation.toml");
        this->width = tbl["width"].ref<double>();
        auto loadIntVector = [] (toml::table tbl, string key) {
            vector<int> holder;
            auto tomlArr = tbl[key];
            if (toml::array* arr = tomlArr.as_array()) {
                for (toml::node& elm : *arr) {
                    elm.visit([&holder](auto&& el) noexcept {
                        if constexpr (toml::is_integer<decltype(el)>) {
                            int64_t s (el);
                            holder.push_back(s);
                        }
                    });
                }
            }
            return holder;
        };
        vector<int> frameLenghtInts= loadIntVector(tbl, "frame_lengths");
        for (int i = 0; i < frameLenghtInts.size(); i++) {
            frameLengths.push_back(frameLenghtInts[i] / 1000.0f);
        }
    } catch (const toml::parse_error& err) {
        printf("Unable to parse animation.toml [%s]\n", aniPath.c_str());
        frameLengths = vector<float>(1, 1.0f);
        this->width = 1.0f;
    }

    auto vs = ListDirectory(aniPath);
    for (int i = 0; i < vs.size(); i++) {
        if (vs[i] != "animation.toml") {
            frames.push_back(LoadTexture((aniPath + "/" + vs[i]).c_str()));
        }
    }

    totalTime = 0;
    for (int i = 0; i < frameLengths.size(); i++) {
        totalTime += frameLengths[i];
    }
}

Texture2D Animation::getFrame() {
    float t = fmod(GetTime(), totalTime);
    float sum = 0;
    for (int i = 0; i < frameLengths.size(); i++) {
        sum += frameLengths[i];
        if (t < sum) {
            return frames[i];
        }
    }
    Texture2D aaahh;
    return aaahh;
}

Player::Player(float x, float y) {
    pos = Vector2{x, y};
    vel = Vector2{0, 0};
    size = UNIT;
    state = PlayerState::Standing;
    lastJumped = GetTime();
    lastWallJumped = GetTime();
    lastWalljumped = 0;
    isLeft = true;
    
    anis.push_back(Animation("assets/art/player/standing")); // pANI_STANDING,
    anis.push_back(Animation("assets/art/player/jumping")); // pANI_FALLING,
    anis.push_back(Animation("assets/art/player/standing")); // pANI_CLIMBING,
    anis.push_back(Animation("assets/art/player/walking")); // pANI_WALKING,
    anis.push_back(Animation("assets/art/player/sliding")); // pANI_SLIDING,
}

void Player::draw() {
    Texture2D t;
    Animation a = anis[pANI_STANDING];
    switch (this->state) {
    case PlayerState::Air: a = anis[pANI_FALLING]; break;
    case PlayerState::Climbing: a = anis[pANI_CLIMBING]; break;
    case PlayerState::Running: a = anis[pANI_WALKING]; break;
    case PlayerState::Sliding: a = anis[pANI_SLIDING]; break;
    case PlayerState::Standing: a = anis[pANI_STANDING]; break;
    }
    float scale = size / (float)t.width;
    t = a.getFrame();
    function<int(int)> jabs = [](int i) {
        return i < 0 ? i * -1 : i;
    };
    float size = UNIT * a.getWidth();
    DrawTexturePro(t, Rectangle{0,0,t.width * (isLeft ? -1 : 1),t.height}, Rectangle{pos.x,pos.y,size,size*t.height/t.width}, Vector2{0,0}, 0, WHITE);
}

void updatePlayer(Player* p, GameData *d, float delta) {
    auto movementDisabled = GetTime() - p->lastWallJumped < WALLJUMP_MOVEMENT_COOLDOWN;
    auto effective_max_velocity = d->keys.down.b ? SPRINT_SPEED_MODIFIER * MAX_VELOCITY : MAX_VELOCITY;
    auto effective_accel = d->keys.down.b ? SPRINT_ACCEL_MODIFIER * ACCEL : ACCEL;
    if (p->state == PlayerState::Climbing) {
    } else {
        p->state = PlayerState::Standing;
        if (!movementDisabled) {
            if (d->keys.down.left && !d->keys.down.right) {
                p->isLeft = true;
                p->vel.x -= effective_accel * delta;
                p->vel.x = max(p->vel.x, -effective_max_velocity);
                p->state = PlayerState::Running;
            } else if (d->keys.down.right && !d->keys.down.left) {
                p->isLeft = false;
                p->vel.x += effective_accel * delta;
                p->vel.x = min(p->vel.x, effective_max_velocity);
                p->state = PlayerState::Running;
            }
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
                            if (d->keys.down.up || d->keys.down.down)
                            climbing = true;
                        }
                    }
                } else if (d->map.collisiondata[idx] == COL_ONE_WAY) {
                    auto pb = Rectangle{px+inset/2, py + ps - tiny_colmask, ps-inset, tiny_colmask};
                    auto onewaytile = Rectangle{xt*UNIT, yt*UNIT, UNIT, UNIT/5};
                    if (p->vel.y > 0 && CheckCollisionRecs(pb, onewaytile)) {
                        p->pos.y = yt * UNIT - ps + 1;
                        p->vel.y = 0;
                        grounded = true;
                    }
                } else if (d->map.collisiondata[idx] == COL_DEAD) {
                    auto pb = Rectangle{px, py, ps, ps};
                    auto tile = Rectangle{xt*UNIT, yt*UNIT, UNIT, UNIT};
                    if (p->vel.y > 0 && CheckCollisionRecs(pb, tile)) {
                        d->state = GameState::Failed;
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
    if (p->state == PlayerState::Climbing && (!climbing || d->keys.pressed.b)) {
        p->state = PlayerState::Air;
    }

    if (grounded) {
        p->lastWalljumped = 0;
    }

    bool jumped = false;
    if (d->keys.pressed.a) {
        if (p->state == PlayerState::Climbing) {
            p->vel.y = JUMP_VELOCITY;
            jumped = true;
            p->lastJumped = GetTime();
            p->state = PlayerState::Air;
        } else if (grounded && GetTime() - p->lastJumped > JUMP_COOLDOWN) {
            p->vel.y = JUMP_VELOCITY;
            jumped = true;
            p->lastJumped = GetTime();
        } else if (walled && GetTime() - p->lastJumped > JUMP_COOLDOWN) {
            if (p->lastWalljumped != walled) {
                p->vel.y = JUMP_VELOCITY;
                p->vel.x = - walled * WALLJUMP_VELOCITY;
                jumped = true;
                p->lastJumped = GetTime();
                p->lastWallJumped = p->lastJumped;
                p->lastWalljumped = walled;
                p->isLeft = walled > 0;
            }
        }
    }
    if (d->keys.down.a) {
        if (GetTime() - p->lastJumped < JUMP_EXTENSION_TIME) {
            p->vel.y = JUMP_VELOCITY;
        }
    }

    if (p->state == PlayerState::Standing) {
        p->vel = Vector2Scale(p->vel, FRICTION_FACTOR);
    } else if (p->state == PlayerState::Air) {
        p->vel.x = p->vel.x * AIR_FRICTION_FACTOR;
    } else if (p->state == PlayerState::Climbing) {
        p->vel.x = 0;
        p->vel.y = 0;
        if (d->keys.down.left) p->vel.x -= CLIMB_SPEED;
        if (d->keys.down.right) p->vel.x += CLIMB_SPEED;
        if (d->keys.down.up) p->vel.y -= CLIMB_SPEED;
        if (d->keys.down.down) p->vel.y += CLIMB_SPEED;
    }

    // printf("Pos (%f,%f). Vel(%f,%f) Delta(%f, %f)\n", p->pos.x, d->player.pos.y, d->player.vel.x, d->player.vel.y, d->player.vel.x * delta, d->player.vel.y * delta);
    auto dv = Vector2Scale(p->vel, delta);
    float eps = 0.1;
    if ((d->keys.down.right || jumped) && dv.x > eps && dv.x < 1) { dv.x = 1; }
    if ((d->keys.down.left || jumped) && dv.x < -eps && dv.x > -1) { dv.x = -1; }
    if (jumped && dv.y < -eps && dv.y > -1) { dv.y = -1; }
    p->pos = Vector2Add(p->pos, dv) ;
    d->cam.target = p->pos;
}

void Player::update(GameData *d, float delta) {
    updatePlayer(this, d, delta);
}


void Player::collidesWith(GameData *data, Entity* other) {
    if (KongBarrel* p = dynamic_cast<KongBarrel*>(other)) {
        data->state = GameState::Failed;
    } else if (Kong* p = dynamic_cast<Kong*>(other)) {
        data->state = GameState::Failed;
    }
}

