#ifndef MAP_H_
#define MAP_H_
#include <string>
#include <vector>

#include <raylib.h>
using namespace std;

struct Tileset {
    string root;
    vector<string> tiles;
};
struct Map {
    string path;
    vector<int> tiledata;
    int width;
    int height;
    int specials;
    Tileset tileset;
};
Tileset TilesetLoad(string path);
vector<Texture2D> TilesetTextures(Tileset *t);
#endif
