#include <algorithm>
#include <set>

#include <raylib.h>
#include <raymath.h>

#include "constants.h"
#include "game.h"
#include "map.h"
#include "util.h"


void restartLevel(GameData* d) {
    Level l (d->level);
    *d = l.GenerateGameData();
    d->state = GameState::Running;
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
    if (d->keys.select) {d->state = GameState::Paused;}

    set<pair<Entity*, Entity*>> collisions;
    int size = d->entities.size();
    for (int i = 0; i < size; i++) {
        Entity* e = d->entities[i];
        e->update(d, delta);

        // If we ever have lots of entities, something more effecient will need to be done. BSP?
        Rectangle re = Rectangle{e->pos.x, e->pos.y, e->size, e->size};
        for (int j = 0; j < size; j++) {
            Entity* o = d->entities[j];
            Rectangle ro = Rectangle{o->pos.x, o->pos.y, o->size, o->size};
            if (CheckCollisionRecs(re, ro)) {
                if (o != e) {
                    if (o < e) {
                        collisions.insert(pair<Entity*, Entity*>(o, e));
                    } else {
                        collisions.insert(pair<Entity*, Entity*>(e, o));
                    }
                }
            }
        }
    }

    for (auto c : collisions) {
        c.first->collidesWith(d, c.second);
        c.second->collidesWith(d, c.first);
    }

    auto res = remove_if(d->entities.begin(), d->entities.end(), [](auto const x) { return !x->isValid(); });
    d->entities.erase(res, d->entities.end());
}


void DrawTextureJ(Texture2D texture, float x, float y, float size) {
    float scale = size / (float)texture.width;
    DrawTextureEx(texture, Vector2{x, y}, 0, scale, WHITE);
}

void drawGame(GameData *d) {
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

void fancyDrawText (string msg, int x, int y, bool centered) {
    const char* txt = msg.c_str();
    int size = 50;
    float tw = MeasureText(txt, size);
    float rw = tw;
    if (!centered) {
        tw = 0;
    }
    int padd = 10;
    DrawRectangle(x - tw/2 - padd, y - size/2 - padd, rw + 2 * padd, size + 2 * padd, ColorAlpha(RAYWHITE, 0.5));
    DrawText(txt, x - tw/2, y - size/2, size, BLACK);
};

void handlePauseState(GameData* d) {
    ClearBackground(RAYWHITE);
    static int pauseState = 0;
    static float pauseStateShiftCooldown = 0.2;
    auto fdt = [] (string msg, int x, int y, bool centered, bool selected) {
        auto m2 = msg;
        if (selected) {
            m2 = "[" + m2 + "]";
        }
        fancyDrawText(m2, x, y, centered);

    };
    fancyDrawText("Paused", 400, 100, true);
    fdt("Continue", 200, 160, false, pauseState == 0);
    fdt("Restart", 200, 220, false, pauseState == 1);
    fdt("Title screen", 200, 280, false, pauseState == 2);
    fdt("Quit application", 200, 340, false, pauseState == 3);

    static float lastTime = 0;
    float delta = GetTime() - lastTime;
    lastTime += delta;
    pauseStateShiftCooldown -= delta;
    if (pauseStateShiftCooldown < 0 && (d->keys.up || d->keys.down)) {
        if (d->keys.up) {
            pauseState--;
            if (pauseState < 0) {pauseState = 0;}
        } else if (d->keys.down) {
            pauseState++;
            if (pauseState > 3) {pauseState = 3;}
        }
        pauseStateShiftCooldown = 0.2;
    }

    if (d->keys.a) {
        switch (pauseState) {
        case 0: // Continue
            d->state = GameState::Running;
            break;
        case 1: // Restart

            break;
        case 2: // Title Screen
            break;
        case 3: // Quit Application
            break;
        }

        pauseState = 0;
    } else if (d->keys.b) {
        d->state = GameState::Running;
        pauseState = 0;
    }
}

void handleGameOverScreen (GameData *d) {
    static float gameOverCooldown;
    static float lastTime = 0;
    float delta = GetTime() - lastTime;
    lastTime += delta;
    if (delta < 0.1) {
        gameOverCooldown -= delta;
    } else {
        gameOverCooldown = 0.4;
    }
    fancyDrawText("You Died :(", 400, 225, true);
    if (gameOverCooldown < 0 && (d->keys.a || d->keys.b)) {
        restartLevel(d);
    }
}

void renderDialog(GameData* d) {
    // Draw Rectangle
    DrawRectangle(0, 300, 800, 150, RAYWHITE);
    DrawRectangleLines(10, 310, 780, 130, BLACK);
    DrawText(d->dia.msg.c_str(), 20, 320, 20, BLACK);
}

void updateDialog(GameData* d, float delta) {
    for (Entity* e : d->entities) {
        if (Player* p = dynamic_cast<Player*>(e)) {
            d->cam.target = p->pos;
            d->cam.target = Vector2Add(p->pos, d->dia.offset);
        }
    }

    d->dia.cooldown -= delta;
    if (d->dia.cooldown < 0) {
        if (d->keys.a || d->keys.b) {
            d->dia.cooldown = 0.05 / 3;
        } else {
            d->dia.cooldown = 0.05;
        }
        if (d->dia.waiting) {
            if (d->keys.a || d->keys.b) {
                d->dia.waiting = false;
            } else {
                d->dia.cooldown = -1;
            }
            return;    
        }
        for (int i = 0; i < 10; i++) {
            if (d->dia.moveTo.y != 0) {
                d->dia.moveTo.y -= d->dia.moveTo.z;
                d->dia.offset.y += d->dia.moveTo.z;
                return;
            }
            if (d->dia.moveTo.x != 0) {
                d->dia.moveTo.x -= d->dia.moveTo.z;
                d->dia.offset.x += d->dia.moveTo.z;
                return;
            }
        }
        if (d->dia.idx >= (int)d->dia.data[d->dia.current_dialog].size()) {
            d->state = GameState::Running;
            return;
        }
        if (d->dia.idx == (int)d->dia.data[d->dia.current_dialog].size() - 1) {
            d->dia.waiting = true;
        }
        char c = d->dia.data[d->dia.current_dialog][d->dia.idx++];
        if (c == '|') {
            int idx2 = d->dia.idx;
            while (d->dia.data[d->dia.current_dialog][idx2++] != '|');
            string s (d->dia.data[d->dia.current_dialog], d->dia.idx, idx2 - d->dia.idx - 1);
            d->dia.idx = idx2;

            printf("Special string |%s|\n", s.c_str());
            // handle special string
            if (s == "") {
                d->dia.waiting = true;
            } else if (s.rfind("up", 0) == 0) {
                int arg = stoi(s.substr(3));
                d->dia.moveTo = Vector3{0, -(float)arg, -1};
            } else if (s.rfind("down", 0) == 0) {
                int arg = stoi(s.substr(5));
                d->dia.moveTo = Vector3{0, (float)arg, 1};
            } else if (s == "clear") {
                d->dia.msg = "";
            }

        } else {
            d->dia.msg.push_back(c);
        }
    }
}

// This one doesn't move with player
void drawHUD(GameData *d) {
    if (d->state == GameState::Failed) {
        handleGameOverScreen(d);
    } else if (d->state == GameState::Succeeded) {
        fancyDrawText("You Win :)", 400, 225, true);
    } else if (d->state == GameState::Paused) {
        handlePauseState(d);
    } else if (d->state == GameState::Dialog) {
        renderDialog(d);
    }
}

// This one does
void draw(GameData *d) {
    if (d->state == GameState::Running || d->state == GameState::Failed || d->state == GameState::Dialog) {
        drawGame(d);
    }
}



const float PHYSICS_TIMESTEP = 0.005f;
void doUpdates(GameData *d) {
    static float lastTime = 0;
    float delta = GetTime() - lastTime;
    lastTime += delta;
    if (delta > 0.1) {return;}

    if (d->state == GameState::Running) {
        float updateTime = 0;
        while (updateTime < delta) {
            update(d, PHYSICS_TIMESTEP);
            updateTime += PHYSICS_TIMESTEP;
        }
    } else if (d->state == GameState::Paused) {
    } else if (d->state == GameState::Dialog) {
        updateDialog(d, delta); 
    }
}

void RenderGame(GameData *d) {
    input(d);

    doUpdates(d);

    BeginDrawing();
    BeginMode2D(d->cam);
    draw(d);
    EndMode2D();
    drawHUD(d);
    EndDrawing();

}

Entity::~Entity() {
}
