#include <raylib.h>

#define PHYSAC_IMPLEMENTATION
#include "game.h"
#include "map.h"
#include "util.h"

const float UNIT = 50;

PhysicsBody JCreatePhysicsBody(float x, float y, float w, float h, bool moving) {
    auto p = CreatePhysicsBodyRectangle(Vector2 {x, y}, w, h, w * h * (moving ? 1 : 10) / UNIT);
    p->enabled = moving;
    p->freezeOrient = moving;
    p->dynamicFriction = 0.3;
    p->staticFriction = 0.5;
    printf("Made Physics Body [%f, %f, %f, %f] %s\n", x, y, w, h, moving ? "player" : "wall");
    return p;
}

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
        if (m.collisiondata[i] == COL_SOLID && paintedTiles[i] < 0b11) {
            int x = (i % m.width);
            int i2 = 0;
            bool addX = 0 == (paintedTiles[i] & 0b01);
            while (i2 + x < m.width && m.collisiondata[i + i2] == COL_SOLID) {
                paintedTiles[i + i2] |= 0b01;
                i2++;
            }
            int y = (i / m.width);
            int i3 = 0;
            bool addY = 0 == (paintedTiles[i] & 0b10);
            while (i3 + y < m.height && m.collisiondata[i + m.width * i3] == COL_SOLID) {
                paintedTiles[i + m.width * i3] |= 0b10;
                i3++;
            }
            printf("i2: %d, i3: %d\n", i2, i3);
            const int diff = 10;
            if (addX && addY && i3 == 1 && i2 == 1) { // Solo Tile
                PhysicsBody b = JCreatePhysicsBody(x * UNIT + (UNIT) / 2, y * UNIT + UNIT/2, UNIT, UNIT, false);
                gd.bodies.push_back(b);
                continue;
            }
            if (addX) {
                PhysicsBody b1 = JCreatePhysicsBody(x * UNIT + (UNIT*i2) / 2, y * UNIT + UNIT/2, UNIT * i2-diff, UNIT, false);
                gd.bodies.push_back(b1);
            }
            if (addY) {
                PhysicsBody b2 = JCreatePhysicsBody(x * UNIT + (UNIT) / 2, y * UNIT + (UNIT*i3)/2, UNIT, UNIT * i3-diff, false);
                gd.bodies.push_back(b2);
            }
        }
    }
    for (int i = 0; i < (int)m.tiledata.size(); i++) {
        if (m.tiledata[i] == tileCount) { // special 1 is player
            PhysicsBody body = JCreatePhysicsBody((i % m.width) * UNIT + UNIT/2, (i / m.width) * UNIT + UNIT/2, UNIT, UNIT, true);
            gd.player = Player { body };
        }
    }


    return gd;
}

void input(GameData *d) {

    d->keys.up     = IsKeyDown(KEY_W) || IsKeyDown(KEY_K) || IsKeyDown(KEY_UP);
    d->keys.down   = IsKeyDown(KEY_S) || IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN);
    d->keys.left   = IsKeyDown(KEY_A) || IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT);
    d->keys.right  = IsKeyDown(KEY_D) || IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT);
    d->keys.a      = IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    d->keys.b      = IsKeyPressed(KEY_LEFT_SHIFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    d->keys.start  = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q);
    d->keys.select = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E);

}

void update(GameData *d) {
    static float lastTime = 0;
    float delta = GetTime() - lastTime;
    lastTime += delta;

    UpdatePhysics();
    if (d->keys.start) {SetScreen(SCREEN_EDITOR);}

    const float VELOCITY = 1.0f;
    const float JUMP_VELOCITY = -1.5f;
    const float FORCE = 6000;
    float f = FORCE * (1 - abs((d->player.body->velocity.x / VELOCITY)));
    if (d->keys.left && !d->keys.right) {
        PhysicsAddForce(d->player.body, Vector2{-f, 0});
    } else if (d->keys.right && !d->keys.left) {
        PhysicsAddForce(d->player.body, Vector2{f, 0});
    }

    d->player.body->dynamicFriction = d->keys.left || d->keys.right ? 0.1 : 0.4;
    
    if (d->keys.a) {
        if (d->player.body->isGrounded) {
            d->player.body->velocity.y = JUMP_VELOCITY;
        } else if (d->player.body->isWallSliding) {
            d->player.body->velocity.y = JUMP_VELOCITY;
            d->player.body->velocity.x = - d->player.body->isWallSliding * VELOCITY / 2;
        }
    }


    d->cam.target = d->player.body->position;

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
    if (d->player.body->isWallSliding) {
        c = RED;
    }
    if (d->player.body->isGrounded) {
        c = YELLOW;
    }
    DrawRectangle(d->player.body->position.x - UNIT/2, d->player.body->position.y - UNIT/2, UNIT, UNIT, c);
     
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
