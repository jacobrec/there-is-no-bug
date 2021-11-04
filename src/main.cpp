#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <raylib.h>


#include "editor.h"
#include "game.h"
#include "util.h"


using namespace std;

const int screenWidth = 800;
const int screenHeight = 450;

Screen s = SCREEN_EDITOR;
EditorData ed;
GameData gd;

void SetScreen(Screen ns) {
    if (s == SCREEN_EDITOR && ns == SCREEN_GAME) {
        gd = InitGame(ed.m);
    }
    s = ns;
}

int main(void) {

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "there is no bug");
    SetTargetFPS(60);

    ed = InitEditor();

    while (!WindowShouldClose()) {
        switch (s) {
        case SCREEN_GAME:
            RenderGame(&gd);
            break;
        case SCREEN_EDITOR:
            BeginDrawing();
            RenderEditor(&ed);
            EndDrawing();
            break;
        }
    }

    return 0;
}
