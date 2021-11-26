#ifndef PEDITOR_H_
#define PEDITOR_H_
#include <string>
#include <vector>
#include <raylib.h>

#include "extras/toml.hpp"

using namespace std;

struct PEditorData {
    string dir;
    toml::table tbl;
    vector<Texture2D> frames;
    vector<float> frameLength;
    float aWidth;
    float aHeight;
};

PEditorData InitPEditor(string dir);
void RenderPEditor(PEditorData *d);

#endif
