#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>

using namespace std;

vector<string> ListDirectory(string path);
string Join(vector<string> l, string delim);

#endif
