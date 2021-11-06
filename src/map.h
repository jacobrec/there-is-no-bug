#ifndef MAP_H_
#define MAP_H_
#include <string>
#include <vector>

#include <raylib.h>
using namespace std;

enum CollisionType {
    COL_NONE,
    COL_SOLID,
    COL_CLIMB,
    COL_ONE_WAY,
    COLLISION_TYPE_COUNT
};

struct Tileset {
    string root;
    vector<string> tiles;
};
struct Map {
    string path;
    vector<int> tiledata;
    vector<int> collisiondata;
    int width;
    int height;
    int specials;
    Tileset tileset;
};
Tileset TilesetLoad(string path);
vector<Texture2D> TilesetTextures(Tileset *t);

void SaveMap(Map *m);
Map LoadMap(string path);

#endif
