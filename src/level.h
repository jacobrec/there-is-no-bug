#pragma once

#include <vector>
#include <string>

using namespace std;

struct GameData;
class Level {
public:
    string levelFile;
    string name;
    string map;
    vector<string> specials;
    vector<string> dialogs;

public:
    Level(string levelFile);
    GameData GenerateGameData();
};

