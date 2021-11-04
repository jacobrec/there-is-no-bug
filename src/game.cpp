#include <raylib.h>
#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

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
    for (int i = 0; i < (int)m.tiledata.size(); i++) {
        if (m.tiledata[i] == tileCount) { // special 1 is player
            gd.player = Player {};
            gd.player.size = UNIT;
            gd.player.pos.x = (i % m.width) * UNIT;
            gd.player.pos.x = (i / m.width) * UNIT;
        }
    }
    return gd;
}

void input(GameData *d) {

    d->keys.up     = IsKeyDown(KEY_W) || IsKeyDown(KEY_K) || IsKeyDown(KEY_UP);
    d->keys.down   = IsKeyDown(KEY_S) || IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN);
    d->keys.left   = IsKeyDown(KEY_A) || IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT);
    d->keys.right  = IsKeyDown(KEY_D) || IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT);
    d->keys.a      = IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    d->keys.b      = IsKeyDown(KEY_LEFT_SHIFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    d->keys.start  = IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_Q);
    d->keys.select = IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_E);

}

void update(GameData *d) {
    static float lastTime = 0;
    float delta = GetTime() - lastTime;
    lastTime += delta;

    if (d->keys.start) {SetScreen(SCREEN_EDITOR);}
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

    DrawRectangle(d->player.pos.x, d->player.pos.y, d->player.size, d->player.size, ORANGE);
     
}


void RenderGame(GameData *d) {
    input(d);
    update(d);

    BeginDrawing();
    BeginMode2D(d->cam);
    draw(d);
    EndMode2D();
    EndDrawing();

}
