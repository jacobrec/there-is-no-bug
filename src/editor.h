#ifndef EDITOR_H_
#define EDITOR_H_
#include <string>
#include <vector>
#include <raylib.h>

#include "map.h"
#include "util.h"
#include "level.h"

using namespace std;

struct EditorData {
    Level l;
    Map m;
    int tilesetChoice;
    int mapChoice;
    float zoom;
    float panOffsetX;
    float panOffsetY;
    int selectedTile;
    int selectedCollision;
    bool showGrid;
    bool collisionMode;
    vector<string> tilesetChoices;
    vector<Texture2D> images;
    bool shouldPlay;
};

EditorData InitEditor(int level);
void RenderEditor(EditorData *d);

#endif
