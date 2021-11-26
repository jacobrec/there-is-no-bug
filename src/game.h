#pragma once
#include <vector>
#include <raylib.h>

#include "map.h"
#include "level.h"

using namespace std;

enum class PlayerState {
    Air, Sliding, Running, Standing, Climbing
};
enum class GameState {
    Failed, Running, Succeeded, Paused, Dialog
};
struct GameData;
class Entity {
public:
    Vector2 pos;
    float size;
    virtual ~Entity();
    virtual void update(GameData* d, float delta) {};
    virtual void draw() {};
    virtual bool isValid() { return true; };
    virtual void collidesWith(GameData* d, Entity* other) { };
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
struct Keys {
    Keymap down;
    Keymap pressed;
};

struct DialogState {
    string msg;
    int current_dialog;
    int idx;
    Vector2 offset;
    Vector3 moveTo;
    float cooldown;
    bool waiting;
    vector<string> data;
};

struct GameData {
    string level;
    GameData* resetState;
    Map map;
    Camera2D cam;
    vector<Entity*> entities;
    vector<Texture2D> images;
    Keys keys;
    GameState state;
    DialogState dia;
    vector<string> specials;
};
extern GameData resetState;

GameData InitGame(Map m);
void RenderGame(GameData *d);

class Animation {
    vector<Texture2D> frames;
    vector<float> frameLengths;
    double totalTime;
    double width;
public:
    Animation(string aniPath);
    Texture2D getFrame();
    double getWidth() const { return width; };
};
//////////////
// Entities //
//////////////

enum PlayerAnimationIndexes {
    pANI_STANDING,
    pANI_FALLING,
    pANI_CLIMBING,
    pANI_WALKING,
    pANI_RUNNING,
    pANI_SLIDING,
};
class Player : public Entity {
public:
    Player(float x, float y);
    Vector2 vel;
    PlayerState state;
    float lastJumped;
    // TODO: rename on of these two variables
    float lastWallJumped; // this is the number of seconds since you last walljumped
    int lastWalljumped; // this is the direction of the last wall jumped
    bool isLeft;
    vector<Animation> anis;
    void update(GameData* d, float delta) override;
    void draw() override;
    void collidesWith(GameData* d, Entity* other) override;

};

class Kong : public Entity {
public:
    Kong(float x, float y);
    int animationState; // 0 is still, 1 is beating chest, 2 is throwing
    float animationTime;
    void update(GameData* d, float delta) override;
    void draw() override;
};

class KongBarrel : public Entity {
public:
    KongBarrel(float x, float y, int type);
    int type;
    Vector2 vel;
    bool preferLeft;
    int bounces;

    void update(GameData* d, float delta) override;
    void draw() override;
    bool isValid() override;
};

enum class Effects {
    Win, Dialog, SwitchMap
};

class EffectEntity : public Entity {
    int triggered;
    vector<pair<Effects, int>> effects;
public:
    EffectEntity(float x, float y, vector<pair<Effects, int>> data);
    void draw() override;
    void collidesWith(GameData* d, Entity* other) override;
};


void loadEntities(GameData* d, Map &m);
void restartLevel(GameData* d);
