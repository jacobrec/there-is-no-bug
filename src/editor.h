#ifndef EDITOR_H_
#define EDITOR_H_
#include <string>
#include <vector>
#include <raylib.h>

#include "map.h"
#include "util.h"

using namespace std;

struct EditorData {
    Map m;
    int tilesetChoice;
    int mapChoice;
    float zoom;
    float panOffsetX;
    float panOffsetY;
    int selectedTile;
    bool showGrid;
    vector<string> tilesetChoices;
    vector<Texture2D> images;
};

EditorData InitEditor();
void RenderEditor(EditorData *d);

#endif
