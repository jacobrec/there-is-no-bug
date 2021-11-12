#include <raylib.h>
#include "game.h"
#include "constants.h"

EffectEntity::EffectEntity(float x, float y, vector<pair<Effects, int>> data) {
    effects = data;
    pos = Vector2{x, y};
    size = UNIT;
    triggered = effects.size();
}

void EffectEntity::draw() {
    DrawRectangle(this->pos.x, this->pos.y, UNIT, UNIT, ColorAlpha(WHITE, 0.5));
}

void EffectEntity::collidesWith(GameData *data, Entity* other) {
    if (triggered > 0) {
        if (Player* p = dynamic_cast<Player*>(other)) {
            auto e = effects[effects.size() - triggered];
            auto effect = e.first;
            int d = e.second;
            printf("Trigger from %X: %d. This implies %d\n", this, triggered, effect);
            switch (effect) {
            case Effects::Win:
                data->state = GameState::Succeeded;
                break;
            case Effects::SwitchMap: {
                Level l (data->level);
                l.currentMap = 1;
                *data = l.GenerateGameData();
                restartLevel(data);
                data->state = GameState::Dialog;
            } break;
            case Effects::Dialog:
                data->dia.current_dialog = d;
                data->dia.idx = 0;
                data->dia.msg = "";
                data->dia.offset = Vector2 {0,0};
                data->dia.moveTo = Vector3 {0,0,0};
                data->state = GameState::Dialog;
                break;
            }
            triggered--;
        }
    }
}
