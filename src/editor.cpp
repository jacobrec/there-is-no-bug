#include <algorithm>
#include <string>
#include <iterator>
#include <vector>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include "map.h"
#include "editor.h"
#include "util.h"

void save(EditorData* d) {
    SaveMap(&d->m);
}
void run() {
    SetScreen(SCREEN_GAME);
}

EditorData InitEditor(int level) {
    if (ListDirectory("assets/maps/").size() == 0) {
        printf("No maps found. Unable to load editor. Please create a map file: $ touch assets/maps/mapname.map");
    }
    
    Level l ("assets/levels/" + to_string(level) + ".toml");
    Map m = LoadMap(ListDirectory("assets/maps/")[0]);
    EditorData ed = EditorData {
        l,
        m, // Map m
        0, // int tilesetChoice
        0, // int mapChoice
        1.0, // float zoom
        0.0, // Pan offset X
        0.0, // Pan offset y
        0, // int selectedTile
        0, // int selectedCollision
        true, // bool showGrid
        false, // bool collisionMode
        ListDirectory(string("assets/art/tilesets")), // vector<string> tilesetChoices;
        TilesetTextures(&m.tileset),
        false, // bool shouldPlay
    };
    auto res = find(ed.tilesetChoices.begin(), ed.tilesetChoices.end(), m.tileset.root);
    ed.tilesetChoice = res ==  ed.tilesetChoices.end() ? 0 : res - ed.tilesetChoices.begin();
    return ed;
}


void RenderEditor(EditorData *d) {
    static bool tilesetDropped = false;
    auto tilesetChoicesString = Join(d->tilesetChoices, string(";"));
    auto mapChoicesString = Join(ListDirectory("assets/maps/"), string(";"));
    
    ClearBackground(RAYWHITE);

    float tilescale = d->images[0].width * 5 / 8;
    float tilesize = d->images[0].width * tilescale;

    auto drawCollisionTile = [&tilesize](float x, float y, int i, float zoom) {
        Color c[10] = {WHITE, DARKBLUE, GRAY, ORANGE, BLUE, GREEN, YELLOW, PINK, PURPLE, BEIGE};
        DrawRectangle(x, y, tilesize*zoom, tilesize*zoom, c[i]);
    };
    auto drawTile = [&tilescale, &d, &tilesize](float x, float y, int i, float zoom, int alpha) {
        Color c;
        if (i < (int)d->images.size()) {
            c = WHITE; c.a = alpha;
            DrawTextureEx(d->images[i], Vector2{x, y}, 0, tilescale * zoom, c);
        } else {
            c = ORANGE; c.a = alpha;
            DrawEllipse(x+tilesize/2*zoom, y+tilesize/2*zoom, tilesize/3*zoom, tilesize/3*zoom, c);
            char text[2] = {(char)(48 + i - (int)d->images.size()), 0};
            c = GRAY; c.a = alpha;
            DrawText(text, x+tilesize/2*zoom, y+tilesize/3*zoom, tilesize / 3*zoom, c);
        }
    };

    //////////////////
    // Main Tilemap
    //////////////////
    for (int i = 0; i < (int)d->m.tiledata.size(); i++) {
        float x = d->panOffsetX + (i % d->m.width) * (tilesize * d->zoom);
        float y = d->panOffsetY + (i / d->m.width) * (tilesize * d->zoom) + 50;
        if (d->collisionMode) {
            drawCollisionTile(x, y, d->m.collisiondata[i], d->zoom);
            drawTile(x, y, d->m.tiledata[i], d->zoom, 100);
        } else {
            drawTile(x, y, d->m.tiledata[i], d->zoom, 255);
        }
        auto selectangle = Rectangle{x, y, tilesize*d->zoom+1, tilesize*d->zoom+1};
        if (d->showGrid) {
            DrawRectangleLines(selectangle.x, selectangle.y, selectangle.width, selectangle.height, GRAY);
        }
        if (CheckCollisionPointRec(GetMousePosition(), selectangle)) {
            if (d->collisionMode) {
                drawCollisionTile(x, y, d->selectedCollision, d->zoom);
            } else {
                drawTile(x, y, d->selectedTile, d->zoom, 125);
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                if (d->collisionMode) {
                    d->m.collisiondata[i] = d->selectedCollision;
                } else {
                    d->m.tiledata[i] = d->selectedTile;
                }
            }
        }
    }

    bool keyUp    = IsKeyDown(KEY_W) || IsKeyDown(KEY_K) || IsKeyDown(KEY_UP);
    bool keyDown  = IsKeyDown(KEY_S) || IsKeyDown(KEY_J) || IsKeyDown(KEY_DOWN);
    bool keyLeft  = IsKeyDown(KEY_A) || IsKeyDown(KEY_H) || IsKeyDown(KEY_LEFT);
    bool keyRight = IsKeyDown(KEY_D) || IsKeyDown(KEY_L) || IsKeyDown(KEY_RIGHT);
    if (keyUp && !keyDown) {
        d->panOffsetY += 1;
    } else if (keyDown && !keyUp) {
        d->panOffsetY -= 1;
    }
    if (keyLeft && !keyRight) {
        d->panOffsetX += 1;
    } else if (keyRight && !keyLeft) {
        d->panOffsetX -= 1;
    }


    DrawRectangle(650, 0, 150, 450, RAYWHITE);
    DrawLine(0, 50, 650, 50, GRAY); // Topbar
    DrawLine(650, 0, 650, 450, GRAY); // Sidebar main divider
    DrawLine(650, 365, 800, 365, GRAY); // Sidebar bottom divider
    DrawLine(650, 80, 800, 80, GRAY); // Sidebar top divider

    //////////////////
    // Right Tile picker
    //////////////////
    int tilesPerLine = 3;
    float tilePadding = 5;
    int len = d->collisionMode ? COLLISION_TYPE_COUNT : (int)d->images.size()+d->m.specials;
    for (int i = 0; i < len; i++) {
        float x = (i % tilesPerLine) * (tilePadding + tilesize) + 660;
        float y = (i / tilesPerLine) * (tilePadding + tilesize) + 100;
        if (d->collisionMode) {
            drawCollisionTile(x, y, i, 1.0);
        } else {
            drawTile(x, y, i, 1.0, 255);
        }
        auto selectangle = Rectangle{x - tilePadding/2, y - tilePadding/2, tilesize + tilePadding, tilesize + tilePadding};
        if ((i == d->selectedTile && !d->collisionMode) || (d->collisionMode && i == d->selectedCollision)) {
            DrawRectangleLines(selectangle.x, selectangle.y, selectangle.width, selectangle.height, GRAY);
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), selectangle)) {
            if(d->collisionMode) {
                d->selectedCollision = i;
            } else {
                d->selectedTile = i;
            }
        }
    }

    d->mapChoice = GuiComboBox(Rectangle{10, 10, 150, 30}, mapChoicesString.c_str(), d->mapChoice);
    if (GuiButton(Rectangle{170, 10, 80, 30}, "Switch Map")) {
    }



    //////////////////
    // Topbar Buttons
    //////////////////
    const float zoomInc = 1.2;
    const float lowerZoom = 0.3;
    auto zoomFn = [&d, &lowerZoom] (float amount) {
        d->zoom *= amount;
        if (d->zoom < lowerZoom) {
            d->zoom = lowerZoom;
        }
    };
    GuiLabel(Rectangle{407, 5, 40, 20}, "Zoom");
    if (GuiButton(Rectangle{400, 25, 20, 20}, "+")) {zoomFn(zoomInc);}
    if (GuiButton(Rectangle{420, 25, 20, 20}, "-")) {zoomFn(1/zoomInc);}
    float mouseWheel = GetMouseWheelMove();
    if (mouseWheel != 0) {zoomFn(pow(zoomInc, mouseWheel));}

    GuiLabel(Rectangle{465, 5, 50, 10}, "Run");
    if (GuiButton(Rectangle{450, 20, 50, 25}, "#134#")) {run();}
    GuiLabel(Rectangle{510, 5, 50, 10}, "Save/Run");
    if (IsKeyPressed(KEY_R)|| GuiButton(Rectangle{500, 20, 70, 25}, "#13#")) {save(d); run();}
    GuiLabel(Rectangle{580, 5, 50, 10}, "Save");
    if (GuiButton(Rectangle{570, 20, 50, 25}, "#2#")) {save(d);}


    d->showGrid = GuiCheckBox(Rectangle{320, 10, 10, 10}, "Grid", d->showGrid);
    d->collisionMode = GuiCheckBox(Rectangle{320, 30, 10, 10}, "Collisions", d->collisionMode);
    if (IsKeyPressed(KEY_SPACE)) {
        d->collisionMode = !d->collisionMode;
    }



    //////////////////
    // Bottom Spinners
    //////////////////
    int ow = d->m.width;
    int oh = d->m.height;
    GuiSpinner(Rectangle{700, 370, 80, 20}, "Special", &d->m.specials, 0, 10, false);
    GuiSpinner(Rectangle{700, 395, 80, 20}, "Width", &d->m.width, 0, 100, false);
    GuiSpinner(Rectangle{700, 420, 80, 20}, "Height", &d->m.height, 0, 100, false);
    if (ow != d->m.width || oh != d->m.height) {
        vector<int> oldtiledata = d->m.tiledata;
        vector<int> newtiledata = vector<int>(d->m.width * d->m.height, 0);
        vector<int> oldcoldata = d->m.tiledata;
        vector<int> newcoldata = vector<int>(d->m.width * d->m.height, 0);
        for (int y = 0; y < d->m.height && y < oh; y++) {
            for (int x = 0; x < d->m.width && x < ow; x++) {
                newtiledata[x + y * d->m.width] = oldtiledata[x + y * ow];
                newcoldata[x + y * d->m.width] = oldcoldata[x + y * ow];
            }
        }
        d->m.tiledata = newtiledata;
        d->m.collisiondata = newcoldata;
    }


    //////////////////
    // Top Tileset picker
    //////////////////
    if (GuiButton(Rectangle{660, 55, 130, 20}, "Load Tileset")) {
        d->m.tileset = TilesetLoad(d->tilesetChoices[d->tilesetChoice]);
        d->images = TilesetTextures(&d->m.tileset);
    }
    if (GuiDropdownBox(Rectangle{660, 10, 130, 40}, tilesetChoicesString.c_str(), &d->tilesetChoice, tilesetDropped)) tilesetDropped = !tilesetDropped;
}

