#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <raylib.h>

#define PHYSAC_IMPLEMENTATION
#include "extras/physac.h"

#include "editor.h"


using namespace std;

const int screenWidth = 800;
const int screenHeight = 450;

enum Screens {
    SCREEN_EDITOR,
    SCREEN_GAME,
};


int main(void) {

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [physac] example - physics movement");

    EditorData ed = InitEditor();

    while (!WindowShouldClose()) {
        BeginDrawing();
        RenderEditor(&ed);
        EndDrawing();
    }

    return 0;
}
