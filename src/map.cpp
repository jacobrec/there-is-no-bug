#include <string>
#include <vector>

#include <string.h>

#include <raylib.h>
#include "util.h"
#include "map.h"
using namespace std;

// Map stuff


Tileset TilesetLoad(string path) {
    string fullpath ("assets/art/tilesets/");
    vector<string> values = ListDirectory(fullpath + path);
    return Tileset {path, values};
}

vector<Texture2D> TilesetTextures(Tileset *t) {
    vector<Texture2D> values;
    for (auto s : t->tiles) {
        values.push_back(LoadTexture(("assets/art/tilesets/" + t->root + "/" + s).c_str()));
    }
    return values;
}
