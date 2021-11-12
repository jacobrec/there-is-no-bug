#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <raylib.h>
#include <string.h>


#include "level.h"
#include "editor.h"
#include "game.h"
#include "util.h"


using namespace std;

const int screenWidth = 800;
const int screenHeight = 450;

Screen s = SCREEN_EDITOR;
GameData gd;
EditorData* ed;

void SetScreen(Screen ns) {
    if (ns == SCREEN_GAME && s == SCREEN_EDITOR) {
        gd = ed->l.GenerateGameData();
    }
    s = ns;
}

void loadLevel(int num) {
    Level l ("assets/levels/" + to_string(num) + ".toml");
    gd = l.GenerateGameData();
    s = SCREEN_GAME;
}

int main(int argc, char *argv[]) {

#define ASSERT_ARGS_COUNT(count) if (argc != count) { printf("Expected %d arguments but got %d\n", count, argc); return -1; }
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "there is no bug");
    SetTargetFPS(40);
    EditorData e1 = InitEditor(1);
    ed = &e1;

    if (argc == 1) {
        loadLevel(1);
    } else if (strcmp(argv[1], "edit") == 0) {
        ASSERT_ARGS_COUNT(3);
        string s = "assets/levels/";
        Level l (s + argv[2] + ".toml");
        e1 = InitEditor(stoi(argv[2]));
        s = SCREEN_EDITOR;
    } else {
        ASSERT_ARGS_COUNT(2);
        e1 = InitEditor(stoi(argv[1]));
        loadLevel(stoi(argv[1]));
    }


    Level l ("assets/levels/1.toml");
    printf("Level: %s is using map %s\n", l.name.c_str(), l.maps[0].c_str());


    while (!WindowShouldClose()) {
        switch (s) {
        case SCREEN_GAME:
            RenderGame(&gd);
            break;
        case SCREEN_EDITOR:
            BeginDrawing();
            RenderEditor(ed);
            EndDrawing();
            break;
        }
    }

    CloseWindow();

    return 0;
}
