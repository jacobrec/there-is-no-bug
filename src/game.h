#pragma once
#include <vector>
#include <raylib.h>

#include "map.h"

using namespace std;

enum class PlayerState {
    Air, Sliding, Running, Standing, Climbing
};
enum class GameState {
    Failed, Running, Succeeded, Paused
};

class Entity {
public:
    Vector2 pos;
    float size;
    virtual ~Entity();
    virtual void update(void* d, float delta) {};
    virtual void draw() {};
    virtual bool isValid() { return true; };
    virtual void collidesWith(void* d, Entity* other) { };
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
    vector<Entity*> entities;
    vector<Texture2D> images;
    Keymap keys;
    GameState state;
};

GameData InitGame(Map m);
void RenderGame(GameData *d);

//////////////
// Entities //
//////////////

class Player : public Entity {
public:
    Player(float x, float y);
    Vector2 vel;
    PlayerState state;
    float lastJumped;
    int lastWalljumped;
    void update(void* d, float delta) override;
    void draw() override;
    void collidesWith(void* d, Entity* other) override;

};

class Kong : public Entity {
public:
    Kong(float x, float y);
    int animationState; // 0 is still, 1 is beating chest, 2 is throwing
    float animationTime;
    void update(void* d, float delta) override;
    void draw() override;
};

class KongBarrel : public Entity {
public:
    KongBarrel(float x, float y, int type);
    int type;
    Vector2 vel;
    bool preferLeft;
    int bounces;

    void update(void* d, float delta) override;
    void draw() override;
    bool isValid() override;
};

class WinCondition : public Entity {
public:
    WinCondition(float x, float y);
    void draw() override;
};
