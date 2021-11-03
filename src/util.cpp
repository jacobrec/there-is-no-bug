#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>

#include <string.h>

#include <raylib.h>

using namespace std;

// Utility Functions
vector<string> ListDirectory(string path) {
    vector<string> values;
    int count;
    char** filenames = GetDirectoryFiles(path.c_str(), &count);
    for (int i = 0; i < count; i ++) {
        if (strcmp(filenames[i], "..") == 0 || strcmp(filenames[i], ".") == 0) {
        } else {
            values.push_back(string(filenames[i]));
        }
    }
    ClearDirectoryFiles();
    sort(values.begin(), values.end());
    return values;
}

string Join(vector<string> l, string delim) {
    ostringstream imploded;
    copy(l.begin(), l.end(), ostream_iterator<string>(imploded, delim.c_str()));
    auto res = imploded.str();
    int joinedLen = res.length();
    res[joinedLen - 1] = '\0';
    return res;
}
