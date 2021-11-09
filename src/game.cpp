#include <algorithm>

#include <raylib.h>
#include <raymath.h>

#include "constants.h"
#include "game.h"
#include "map.h"
#include "util.h"




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

    auto isSpecial = [&tileCount](int num, int tiledata) {
        return tiledata == tileCount + num;
    };

    for (int i = 0; i < (int)m.tiledata.size(); i++) {
        float x = (i % m.width) * UNIT;
        float y = (i / m.width) * UNIT;
        if (isSpecial(0, m.tiledata[i])) { // special 0 is player
            Player* p = new Player(x, y);
            gd.entities.push_back(p);
        } else if (isSpecial(1, m.tiledata[i])) { // special 1 is Kong
            Kong* k = new Kong(x, y);
            gd.entities.push_back(k);
        }
    }
    ReloadConstants();


    return gd;
}

void input(GameData *d) {

    d->keys.up     = IsKeyDown(KEY_W) || IsKeyDown(KEY_K) || IsKeyDown(KEY_UP);
    d->keys.down   = IsKeyDown(KEY_S) || IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN);
    d->keys.left   = IsKeyDown(KEY_A) || IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT);
    d->keys.right  = IsKeyDown(KEY_D) || IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT);
    d->keys.a      = IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    d->keys.b      = IsKeyDown(KEY_LEFT_SHIFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    d->keys.start  = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q);
    d->keys.select = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);

    if (DEBUG) {
        if (IsKeyPressed(KEY_R)) {
            ReloadConstants();
        }
    }

}



void update(GameData *d, float delta) {
    if (d->keys.start) {SetScreen(SCREEN_EDITOR);}

    int size = d->entities.size();
    for (int i = 0; i < size; i++) {
        Entity* e = d->entities[i];
        e->update((void*)d, delta);
    }
    auto res = remove_if(d->entities.begin(), d->entities.end(), [](auto const x) { return !x->isValid(); });
    d->entities.erase(res, d->entities.end());
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
            if (DEBUG_DRAWING) {
                if (m.collisiondata[i] == COL_ONE_WAY) {
                    DrawRectangleRec(Rectangle{(float)x, (float)y, UNIT, UNIT/5}, ColorAlpha(ORANGE, 0.5));
                } else if (m.collisiondata[i] == COL_SOLID) {
                    DrawRectangleRec(Rectangle{(float)x, (float)y, UNIT, UNIT}, ColorAlpha(BLUE, 0.5));
                } else if (m.collisiondata[i] == COL_CLIMB) {
                    DrawRectangleRec(Rectangle{(float)x, (float)y, UNIT, UNIT}, ColorAlpha(GRAY, 0.5));
                }
            }

        }
    }

    for (Entity* e : d->entities) {
        e->draw();
    }

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

Entity::~Entity() {
}
