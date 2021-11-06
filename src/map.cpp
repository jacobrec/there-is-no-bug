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

void printMap(Map *m) {
    printf("Map: %s\n", m->path.c_str());
}

void SaveMap(Map *m) {
    unsigned char* bytes = (unsigned char*)malloc(5 + 2 * m->width * m->height + m->path.size() + m->tileset.root.size());
    int byteIdx = 0;
    bytes[byteIdx++] = m->width;
    bytes[byteIdx++] = m->height;
    bytes[byteIdx++] = m->specials;
    bytes[byteIdx++] = m->path.size();
    bytes[byteIdx++] = m->tileset.root.size();
    for (int i = 0; i < bytes[3]; i++) {
        bytes[byteIdx++] = m->path[i];
    }
    for (int i = 0; i < bytes[4]; i++) {
        bytes[byteIdx++] = m->tileset.root[i];
    }
    for (int i = 0; i < bytes[0] * bytes[1]; i++) {
        bytes[byteIdx++] = m->tiledata[i];
        bytes[byteIdx++] = m->collisiondata[i];
    }
    SaveFileData(("assets/maps/" + m->path).c_str(), bytes, byteIdx);

    free(bytes);
}

Map LoadMap(string path) {
    Map m = Map {
        path, // path
        vector<int>(100, 0), // tiledata
        vector<int>(100, COL_NONE), // collisiondata
        10, // width
        10, // height
        1, // specials
        TilesetLoad("bonky_kong"), // tileset
    };

    unsigned int count;
    unsigned char* data = LoadFileData(("assets/maps/"+path).c_str(), &count);
    if (count == 0) {
        UnloadFileData(data);
        return m;
    }
    m.width = data[0];
    m.height = data[1];
    m.specials = data[2];
    int mapNameSize = data[3];
    int tilesetNameSize = data[4];
    string mapname;
    string tilesetname;
    int byteIdx = 5;
    for (int i = 0; i < mapNameSize; i++) {
        mapname.push_back(data[byteIdx++]);
    }
    for (int i = 0; i < tilesetNameSize; i++) {
        tilesetname.push_back(data[byteIdx++]);
    }
    m.tiledata = vector<int>(m.width * m.height, 0);
    m.collisiondata = vector<int>(m.width * m.height, 0);
    for (int i = 0; i < m.width * m.height; i++) {
        m.tiledata[i] = data[byteIdx++];
        m.collisiondata[i] = data[byteIdx++];
    }

    UnloadFileData(data);
    return m;
}
