#ifndef GAME_H_
#define GAME_H_

#include <vector>
#include <raylib.h>

#include "map.h"

using namespace std;

struct Player {
    Vector2 pos;
    Vector2 vel;
    float size;
};

struct Keymap {
    bool left : 1;
    bool right : 1;
    bool up : 1;
    bool down : 1;
    bool a : 1;
    bool b : 1;
    bool start : 1;
    bool select : 1;
};

struct GameData {
    Map map;
    Camera2D cam;
    Player player;
    vector<Texture2D> images;
    Keymap keys;
};

GameData InitGame(Map m);
void RenderGame(GameData *d);
#endif
