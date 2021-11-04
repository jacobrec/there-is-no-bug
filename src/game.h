#pragma once
#include <vector>
#include <raylib.h>

#include "extras/physac.h"

#include "map.h"

using namespace std;

struct Player {
    PhysicsBody body;
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
    vector<PhysicsBody> bodies;
};

GameData InitGame(Map m);
void RenderGame(GameData *d);
