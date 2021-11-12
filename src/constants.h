#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#include <map> // TODO: switch to unordered map for configParser
#include <string>
using namespace std;

const float UNIT = 50;

const bool DEBUG = true;
const bool DEBUG_DRAWING = DEBUG && false;
const bool DEBUG_PRINT_CONFIG = DEBUG && false;

extern float ACCEL;
extern float MAX_VELOCITY;
extern float WALLJUMP_VELOCITY;
extern float JUMP_VELOCITY;
extern float FRICTION_FACTOR;
extern float AIR_FRICTION_FACTOR;
extern float GRAVITY;
extern float SPRINT_SPEED_MODIFIER;
extern float SPRINT_ACCEL_MODIFIER;
extern float JUMP_COOLDOWN;
extern float JUMP_EXTENSION_TIME;
extern float CLIMB_SPEED;
extern float WALLJUMP_MOVEMENT_COOLDOWN;

const string PHYSICS_CONFIG_PATH = "assets/data/physics_constants.txt";



class ConfigParser {
    string filename;
    string data;
    string tok;
    unsigned int dataSize;
    unsigned int idx;
    unsigned int line;
public:
    ConfigParser (string path);
    ~ConfigParser();
    map<string,float> parseFile();
private:
    void reset();
    float parseUnary(map<string, float> &m);
    float parseOp(map<string,float> &m, int precedence);
    string nextToken();
    string peekToken();
    void expect(string tok);
    void error(string msg);
};

void ReloadConstants();
#endif
