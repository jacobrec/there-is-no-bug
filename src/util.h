#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>

using namespace std;

enum Screen {
    SCREEN_EDITOR,
    SCREEN_GAME,
};

void SetScreen(Screen s);

vector<string> ListDirectory(string path);
string Join(vector<string> l, string delim);

#endif
