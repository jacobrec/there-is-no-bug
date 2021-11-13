#include "constants.h"
#include "level.h"
#include "game.h"
#include "extras/toml.hpp"

Level::Level(string levelFile) {
    this->levelFile = levelFile;
    currentMap = 0;
    toml::table tbl;
    try {
        tbl = toml::parse_file(levelFile);
        this->name = tbl["name"].ref<string>();
        auto loadStringVector = [&tbl] (string key) {
            vector<string> holder;
            auto tomlArr = tbl[key];
            if (toml::array* arr = tomlArr.as_array()) {
                for (toml::node& elm : *arr) {
                    elm.visit([&holder](auto&& el) noexcept {
                        if constexpr (toml::is_string<decltype(el)>) {
                            string s (el);
                            holder.push_back(s);
                        }
                    });
                }
            }
            return holder;
        };

        this->specials = loadStringVector("specials");
        this->dialogs = loadStringVector("dialogs");
        this->maps = loadStringVector("maps");
    } catch (const toml::parse_error& err) {
        printf("Unable to parse level [%s]\n", levelFile.c_str());
    }
}



GameData Level::GenerateGameData() {
    Map m = LoadMap(maps[currentMap]);
    GameData gd;
    // Setup Map
    gd.map = m;
    gd.images = TilesetTextures(&m.tileset);

    // Setup dialog
    gd.dia.idx = 0;
    gd.dia.current_dialog = 0;
    gd.dia.offset = Vector2{0,0};
    gd.dia.cooldown = 0.0;
    gd.dia.msg = "";
    gd.dia.data = dialogs;
    gd.dia.waiting = false;
    gd.dia.moveTo = Vector3{0,0,0};

    // Setup Camera
    gd.cam = Camera2D { };
    gd.cam.offset = Vector2{400, 225};
    gd.cam.zoom = 1;
    gd.cam.rotation = 0;


    // Setup other game data
    gd.level = levelFile;
    gd.state = GameState::Dialog;
    gd.resetState = &resetState;

    ReloadConstants();


    // Setup Specials
    gd.specials = specials;
    resetState = gd;
    loadEntities(&gd, m);

    return gd;
}
