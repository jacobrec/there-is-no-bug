#include <algorithm>
#include <string>
#include <iterator>
#include <vector>
#include <iostream>
#include <fstream>
#include <raylib.h>

#include "extras/raygui.h"

#include "tools/player_editor/editor.h"
#include "util.h"

void save(PEditorData* d) {
    toml::array frame_lengths;
    for (float f : d->frameLength) {
        frame_lengths.push_back((int) (f * 1000));
    }
    toml::array hitboxes;
    for (Rectangle r : d->hitboxes) {
        hitboxes.push_back(toml::array{ r.x, r.y, r.width, r.height });
    }
    auto tbl = toml::table{{
       { "width", d->aWidth },
       { "height", d->aHeight },
       { "hitboxes", hitboxes },
       { "frame_lengths", frame_lengths },
   }};
   
   ofstream myfile;
   myfile.open(d->dir + "/animation.toml");
   myfile << tbl << "\n";
   myfile.close();

}

PEditorData InitPEditor(string dir) {
    vector<string> items = ListDirectory(dir);
    vector<double> frameLengths;
    toml::table tbl;
    float aWidth;
    float aHeight;
    try {
        tbl = toml::parse_file(dir + "/animation.toml");
        aWidth = tbl["width"].ref<double>();
        aHeight = tbl["height"].ref<double>();
        // TODO: Load hitboxes
        auto loadIntVector = [] (toml::table tbl, string key) {
            vector<int> holder;
            auto tomlArr = tbl[key];
            if (toml::array* arr = tomlArr.as_array()) {
                for (toml::node& elm : *arr) {
                    elm.visit([&holder](auto&& el) noexcept {
                        if constexpr (toml::is_integer<decltype(el)>) {
                            int64_t s (el);
                            holder.push_back(s);
                        }
                    });
                }
            }
            return holder;
        };
        vector<int> frameLenghtInts= loadIntVector(tbl, "frame_lengths");
        for (int i = 0; i < frameLenghtInts.size(); i++) {
            frameLengths.push_back((double)frameLenghtInts[i] / 1000.0);
        }
    } catch (const toml::parse_error& err) {
        printf("Unable to parse animation.toml [%s]\n", dir.c_str());
        frameLengths = vector<double>(1, 1.0f);
        aWidth = 1.0f;
        aHeight = 1.0f;
    }

    vector<Texture2D> frames;
    auto vs = ListDirectory(dir);
    for (int i = 0; i < vs.size(); i++) {
        if (vs[i] != "animation.toml") {
            frames.push_back(LoadTexture((dir + "/" + vs[i]).c_str()));
        }
    }

    vector<Rectangle> hitboxes;
    PEditorData ed = PEditorData {
        dir,
        tbl,
        frames,
        frameLengths,
        aWidth,
        aHeight,
        hitboxes
    };
    return ed;
}


Rectangle fixRect(Rectangle r) {
    if (r.width < 0) {
        r.width = -r.width;
        r.x -= r.width;
    }

    if (r.height < 0) {
        r.height = -r.height;
        r.y -= r.height;
    }
    return r;
}

int NO_MODE = 0;
int ADD_MODE = 1;
int REMOVE_MODE = 2;
void RenderPEditor(PEditorData *d) {
    static int mode = 0;
    static Vector2 addModeDown = Vector2{-1,-1};
    float totalTime = 0;
    for (int i = 0; i < d->frameLength.size(); i++) {
        totalTime += d->frameLength[i];
    }

    float sum = 0;
    int idx = 0;
    float t1 = fmod(GetTime(), totalTime);
    for (int i = 0; i < d->frameLength.size(); i++) {
        sum += d->frameLength[i];
        if (sum >= t1) {
            idx = i;
            break;
        }
    }

    ClearBackground(ORANGE);
    Texture2D t = d->frames[idx];
    float width = 400;
    float height = 400;
    float x = (GetScreenWidth() - width) / 2;
    float y = (GetScreenHeight() - height) / 2;
    Rectangle frame = Rectangle{x,y,width,height};
    DrawTexturePro(t, Rectangle{0,0,t.width,t.height}, frame,
        Vector2{0,0}, 0, WHITE);
    DrawRectangleLines(x,y,width,height, WHITE);
    for (auto r : d->hitboxes) {
        DrawRectangleRec(r, ColorAlpha(BLACK, 0.8));
    }

    Vector2 m = GetMousePosition();
    if (mode == ADD_MODE) {
        if (!(addModeDown.x == -1 && addModeDown.y == -1)) {
            DrawRectangleRec(fixRect(Rectangle{addModeDown.x, addModeDown.y,
                        m.x - addModeDown.x, m.y - addModeDown.y}),
                ColorAlpha(BLACK, 0.4));

        }
    }


    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(m, frame)) {
        if (mode == ADD_MODE) {
            if (addModeDown.x == -1 && addModeDown.y == -1) {
                addModeDown = m;
            } else {
                Rectangle r = Rectangle{addModeDown.x, addModeDown.y,
                    m.x - addModeDown.x, m.y - addModeDown.y};
                d->hitboxes.push_back(fixRect(r));
                mode = NO_MODE;
                addModeDown = Vector2{-1,-1};
            }
        }
    }

    if (GuiButton(Rectangle{700, 350, 50, 30}, "Add")) {
        mode = ADD_MODE;
    }
    if (GuiButton(Rectangle{700, 400, 50, 30}, "Save")) {
        save(d);
    }
}
