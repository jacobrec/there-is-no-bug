#include <algorithm>
#include <string>
#include <iterator>
#include <vector>
#include <raylib.h>

#include "extras/raygui.h"

#include "tools/player_editor/editor.h"
#include "util.h"

void save(PEditorData* d) {
}

PEditorData InitPEditor(string dir) {
    vector<string> items = ListDirectory(dir);
    vector<float> frameLengths;
    toml::table tbl;
    float aWidth;
    float aHeight;
    try {
        tbl = toml::parse_file(dir + "/animation.toml");
        aWidth = tbl["width"].ref<double>();
        aHeight = tbl["height"].ref<double>();
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
            frameLengths.push_back(frameLenghtInts[i] / 1000.0f);
        }
    } catch (const toml::parse_error& err) {
        printf("Unable to parse animation.toml [%s]\n", dir.c_str());
        frameLengths = vector<float>(1, 1.0f);
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

    PEditorData ed = PEditorData {
        dir,
        tbl,
        frames,
        frameLengths,
        aWidth,
        aHeight,
    };
    return ed;
}


void RenderPEditor(PEditorData *d) {
    ClearBackground(ORANGE);
}
