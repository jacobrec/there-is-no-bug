#pragma once
#include <vector>
#include <raylib.h>

#include "map.h"

using namespace std;

enum class PlayerState {
    Air, Sliding, Running, Standing, Climbing
};
struct Player {
    Vector2 pos;
    Vector2 vel;
    float size;
    PlayerState state;
    float lastJumped;
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
