#include <raylib.h>

#define PHYSAC_IMPLEMENTATION
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
        if (m.collisiondata[i] == COL_SOLID) {
            PhysicsBody b = CreatePhysicsBodyRectangle(Vector2{(i % m.width) * UNIT, (i / m.width) * UNIT}, UNIT, UNIT, 1);
            b->enabled = false;
        }
        if (m.tiledata[i] == tileCount) { // special 1 is player
            gd.player = Player {};
            gd.player.body = CreatePhysicsBodyRectangle(Vector2{(i % m.width) * UNIT, (i / m.width) * UNIT}, UNIT, UNIT, 1);
            gd.player.body->freezeOrient = true;
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

    const float VELOCITY = 0.5f;
    if (d->keys.left && !d->keys.right) {
        d->player.body->velocity.x = -VELOCITY;
    } else if (d->keys.right && !d->keys.left) {
        d->player.body->velocity.x = VELOCITY;
    }
    if (d->keys.a) {
        d->player.body->velocity.y = -VELOCITY * 3;
    }


    d->cam.target = d->player.body->position;

    UpdatePhysicsStep();
    UpdatePhysicsStep();
}

void drawDebugPhysics() {
    int bodiesCount = GetPhysicsBodiesCount();
    for (int i = 0; i < bodiesCount; i++) {
        PhysicsBody body = GetPhysicsBody(i);

        int vertexCount = GetPhysicsShapeVerticesCount(i);
        for (int j = 0; j < vertexCount; j++) {
            Vector2 vertexA = GetPhysicsShapeVertex(body, j);

            int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);
            Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

            DrawLineV(vertexA, vertexB, GREEN);
        }
    }
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

    Color c = ORANGE;
    if (d->player.body->isGrounded) {
        c = YELLOW;
    }
    DrawRectangle(d->player.body->position.x, d->player.body->position.y, UNIT, UNIT, c);
     
}


void RenderGame(GameData *d) {
    input(d);
    update(d);

    BeginDrawing();
    BeginMode2D(d->cam);
    draw(d);
    // drawDebugPhysics();
    EndMode2D();
    EndDrawing();

}
