#include <raylib.h>
#include <raymath.h>

#include "game.h"
#include "map.h"
#include "util.h"

const float UNIT = 50;


GameData InitGame(Map m) {
    GameData gd;
    gd.map = m;
    gd.images = TilesetTextures(&m.tileset);
    gd.cam = Camera2D { };
    gd.cam.offset = Vector2{400, 225};
    gd.cam.zoom = 1;
    gd.cam.rotation = 0;
    int tileCount = m.tileset.tiles.size();
    vector<int> paintedTiles(m.width * m.height, 0);
    for (int i = 0; i < (int)m.tiledata.size(); i++) {
        float x = (i % m.width) * UNIT;
        float y = (i / m.width) * UNIT;
        if (m.collisiondata[i] == COL_SOLID && paintedTiles[i] < 0b11) {
        }
        if (m.tiledata[i] == tileCount) { // special 1 is player
            gd.player.pos = Vector2{x, y};
            gd.player.vel = Vector2{0, 0};
            gd.player.size = UNIT;
            gd.player.state = PlayerState::Standing;
            gd.player.lastJumped = GetTime();
        }
    }


    return gd;
}

void input(GameData *d) {

    d->keys.up     = IsKeyDown(KEY_W) || IsKeyDown(KEY_K) || IsKeyDown(KEY_UP);
    d->keys.down   = IsKeyDown(KEY_S) || IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN);
    d->keys.left   = IsKeyDown(KEY_A) || IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT);
    d->keys.right  = IsKeyDown(KEY_D) || IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT);
    d->keys.a      = IsKeyDown(KEY_SPACE) || IsKeyDown(MOUSE_BUTTON_LEFT);
    d->keys.b      = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(MOUSE_BUTTON_RIGHT);
    d->keys.start  = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q);
    d->keys.select = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);

}

const float ACCEL = 10*UNIT;
const float MAX_VELOCITY = 5*UNIT;
const float JUMP_VELOCITY = -400.0f;
const float FRICTION_FACTOR = 0.9f;
const float AIR_FRICTION_FACTOR = 0.99f;
const float GRAVITY = 35*UNIT;
const float SPRINT_SPEED_MODIFIER = 2.5f;
const float JUMP_COOLDOWN = 0.4f;
const float JUMP_EXTENSION_TIME = 0.2f;

void update(GameData *d, float delta) {

    if (d->keys.start) {SetScreen(SCREEN_EDITOR);}

    auto effective_max_velocity = d->keys.b ? SPRINT_SPEED_MODIFIER * MAX_VELOCITY : MAX_VELOCITY;
    d->player.state = PlayerState::Standing;
    if (d->keys.left && !d->keys.right) {
        d->player.vel.x -= ACCEL * delta;
        d->player.vel.x = max(d->player.vel.x, -effective_max_velocity);
        d->player.state = PlayerState::Running;
    } else if (d->keys.right && !d->keys.left) {
        d->player.vel.x += ACCEL * delta;
        d->player.vel.x = min(d->player.vel.x, effective_max_velocity);
        d->player.state = PlayerState::Running;
    }

    d->player.vel.y += GRAVITY * delta;

    float px = d->player.pos.x;
    float py = d->player.pos.y;
    float ps = d->player.size;
    float inset = UNIT / 5;
    float colmask = UNIT / 5;
    bool grounded = false;
    int walled = 0;
    for (int yt = (int) (py/UNIT) - 1; yt <= (int) (py/UNIT) + 1; yt++) {
        for (int xt = (int) (px/UNIT) - 1; xt <= (int) (px/UNIT) + 1; xt++) {
            int idx = xt + yt * d->map.width;
            if (yt < d->map.height && yt >=0 && xt < d->map.width && xt >= 0 && d->map.collisiondata[idx] == COL_SOLID) {
                auto pb = Rectangle{px+inset/2, py + ps - colmask, ps-inset, colmask};
                auto pt = Rectangle{px+inset/2, py, ps-inset, colmask};
                auto pl = Rectangle{px, py+inset/2, colmask, ps-inset};
                auto pr = Rectangle{px+ps-colmask, py+inset/2, colmask, ps-inset};
                auto tile = Rectangle{xt*UNIT, yt*UNIT, UNIT, UNIT};
                if (CheckCollisionRecs(pb, tile)) { // Collision Bottom
                    d->player.pos.y = yt * UNIT - ps + 1;
                    d->player.vel.y = 0;
                    grounded = true;
                } else if (CheckCollisionRecs(pt, tile)) { // Collision Top
                    d->player.pos.y = yt * UNIT + UNIT;
                    d->player.vel.y = 0;
                } else if (CheckCollisionRecs(pl, tile)) { // Collision Left
                    d->player.pos.x = xt * UNIT + UNIT - 1;
                    d->player.vel.x = max(0.0f, d->player.vel.x);
                    walled = -1;
                } else if (CheckCollisionRecs(pr, tile)) { // Collision Right
                    d->player.pos.x = xt * UNIT - ps + 1;
                    d->player.vel.x = min(0.0f, d->player.vel.x);
                    walled = 1;
                }

            }
        }
    }
    if (!grounded) {
        if (walled != 0) {
            d->player.state = PlayerState::Sliding;
        } else {
            d->player.state = PlayerState::Air;
        }
    }

    bool jumped = false;
    if (d->keys.a) {
        if (grounded && GetTime() - d->player.lastJumped > JUMP_COOLDOWN) {
            d->player.vel.y = JUMP_VELOCITY;
            jumped = true;
            d->player.lastJumped = GetTime();
        } else if (GetTime() - d->player.lastJumped < JUMP_EXTENSION_TIME) {
            d->player.vel.y = JUMP_VELOCITY;
        } else if (walled && GetTime() - d->player.lastJumped > JUMP_COOLDOWN) {
            d->player.vel.y = JUMP_VELOCITY;
            d->player.vel.x = - walled * MAX_VELOCITY / 2;
            jumped = true;
            d->player.lastJumped = GetTime();
        }
    }

    if (d->player.state == PlayerState::Standing) {
        d->player.vel = Vector2Scale(d->player.vel, FRICTION_FACTOR);
    } else if (d->player.state == PlayerState::Air) {
        d->player.vel.x = d->player.vel.x * AIR_FRICTION_FACTOR;
    }

    printf("Pos (%f,%f). Vel(%f,%f) Delta(%f, %f)\n", d->player.pos.x, d->player.pos.y, d->player.vel.x, d->player.vel.y, d->player.vel.x * delta, d->player.vel.y * delta);
    auto dv = Vector2Scale(d->player.vel, delta);
    float eps = 0.1;
    if ((d->keys.right || jumped) && dv.x > eps && dv.x < 1) { dv.x = 1; }
    if ((d->keys.left || jumped) && dv.x < -eps && dv.x > -1) { dv.x = -1; }
    if (jumped && dv.y < -eps && dv.y > -1) { dv.y = -1; }
    d->player.pos = Vector2Add(d->player.pos, dv);
    d->cam.target = d->player.pos;

}


void DrawTextureJ(Texture2D texture, float x, float y, float size) {
    float scale = size / (float)texture.width;
    DrawTextureEx(texture, Vector2{x, y}, 0, scale, WHITE);
}

void draw(GameData *d) {
    int tileCount = (int) d->images.size();
    ClearBackground(RAYWHITE);

    auto m = d->map;
    auto img = d->images;
    for (int i = 0; i < (int)m.tiledata.size(); i++) {
        if (m.tiledata[i] < tileCount) {
            int x = (i % m.width) * UNIT;
            int y = (i / m.width) * UNIT;
            DrawTextureJ(img[m.tiledata[i]], x, y, UNIT);
        }
    }

    Color c = BLUE;
    if (d->player.state == PlayerState::Running) {c = YELLOW;}
    if (d->player.state == PlayerState::Air) {c = ORANGE;}
    if (d->player.state == PlayerState::Sliding) {c = RED;}
    if (d->player.state == PlayerState::Standing) {c = BEIGE;}
    DrawRectangle(d->player.pos.x, d->player.pos.y, UNIT, UNIT, c);

}


const float PHYSICS_TIMESTEP = 0.005f;
void RenderGame(GameData *d) {
    input(d);
    static float lastTime = 0;
    float delta = GetTime() - lastTime;
    lastTime += delta;
    if (delta > 0.1) {return;}

    float updateTime = 0;
    while (updateTime < delta) {
        update(d, PHYSICS_TIMESTEP);
        updateTime += PHYSICS_TIMESTEP;
    }

    BeginDrawing();
    BeginMode2D(d->cam);
    draw(d);
    EndMode2D();
    EndDrawing();

}
