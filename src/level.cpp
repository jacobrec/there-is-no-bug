#include "constants.h"
#include "level.h"
#include "game.h"
#include "extras/toml.hpp"

Level::Level(string levelFile) {
    this->levelFile = levelFile;
    toml::table tbl;
    try {
        tbl = toml::parse_file(levelFile);
        this->name = tbl["name"].ref<string>();
        this->map = tbl["map"].ref<string>();

        auto loadStringVector = [&tbl] (string key, vector<string>& value) {
            auto tomlArr = tbl[key];
            if (toml::array* arr = tomlArr.as_array()) {
                for (toml::node& elm : *arr) {
                    elm.visit([&value](auto&& el) noexcept {
                        if constexpr (toml::is_string<decltype(el)>) {
                            string s (el);
                            value.push_back(s);
                        }
                    });
                }
            }
        };

        vector<string> mspecials;
        vector<string> mdialogs;
        loadStringVector("specials", mspecials);
        loadStringVector("dialogs", mdialogs);
        this->specials = mspecials;
        this->dialogs = mdialogs;


    } catch (const toml::parse_error& err) {
        printf("Unable to parse level [%s]\n", levelFile.c_str());
    }
}


void handleSpecial(float x, float y, GameData* d, string special) {
        if (special == "player") {
            Player* p = new Player(x, y);
            d->entities.push_back(p);
        } else if (special == "enemy kong") {
            Kong* k = new Kong(x, y);
            d->entities.push_back(k);
        } else if (special == "win") {
            WinCondition* k = new WinCondition(x, y);
            d->entities.push_back(k);
        }
}

GameData Level::GenerateGameData() {
    Map m = LoadMap(this->map);
    GameData gd;
    // Setup Map
    gd.map = m;
    gd.images = TilesetTextures(&m.tileset);

    // Setup Camera
    gd.cam = Camera2D { };
    gd.cam.offset = Vector2{400, 225};
    gd.cam.zoom = 1;
    gd.cam.rotation = 0;

    // Setup Specials
    int tileCount = m.tileset.tiles.size();
    if (m.specials != (int) specials.size()) {
        printf("[Warning] number of specials on map is not equal to number of provided specials in level for level %s and map %s\n", name.c_str(), m.path.c_str());
    }

    for (int i = 0; i < (int)m.tiledata.size(); i++) {
        if (m.tiledata[i] >= tileCount && m.tiledata[i] < tileCount + m.specials) {
            float x = (i % m.width) * UNIT;
            float y = (i / m.width) * UNIT;
            handleSpecial(x, y, &gd, specials[m.tiledata[i] - tileCount]);
        }
    }

    // Setup other game data
    gd.level = levelFile;
    gd.state = GameState::Running;
    ReloadConstants();
    return gd;
}
