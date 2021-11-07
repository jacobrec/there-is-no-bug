#include <raylib.h>
#include <ctype.h>
#include <sstream>
#include <functional>
#include "constants.h"

float ACCEL = 10*UNIT;
float MAX_VELOCITY = 5*UNIT;
float JUMP_VELOCITY = -8*UNIT;
float FRICTION_FACTOR = 0.9f;
float AIR_FRICTION_FACTOR = 0.99f;
float GRAVITY = 35*UNIT;
float SPRINT_SPEED_MODIFIER = 2.5f;
float JUMP_COOLDOWN = 0.4f;
float JUMP_EXTENSION_TIME = 0.125f;
float CLIMB_SPEED = 5*UNIT;


ConfigParser::~ConfigParser() {
}
ConfigParser::ConfigParser(string path) {
    filename = path;
}
void ConfigParser::reset() {
    unsigned char* d = LoadFileData(filename.c_str(), &dataSize);
    data = string (reinterpret_cast<char*>(d));
    UnloadFileData(d);
    idx = 0;
    line = 0;
    nextToken();
}
void ConfigParser::error(string msg) {
    printf("[%s] line %d | Error: %s\n", filename.c_str(), line, msg.c_str());
}
void ConfigParser::expect(string tok) {
    string t = nextToken();
    if (tok != t) {
        error("Expected ["+ tok + "]. But got ["+ t + "]");
    }
}

string ConfigParser::nextToken() {
    int i = idx;
    char c = data[idx++];
    string ret = tok;
    bool b = true;
    if (idx >= dataSize) {
        tok = "";
    } else if (c == '*' || c == '-' || c == '*' || c == '+' || c == '=' || c == '\n') {
        tok = string (&c, 1);
    } else if (isalnum(c) || '.' == c || '_' == c) {
        while (isalnum(data[idx]) || '.' == data[idx] || '_' == data[idx]) idx++;
        tok = data.substr(i, idx - i);
    } else {
        b = false;
        nextToken();
    }
    if (DEBUG_PRINT_CONFIG) {
        if (b) {
            printf("next token: [%s]\n", ret.c_str());
        }
    }
    return ret;
}
string ConfigParser::peekToken() {
    if (DEBUG_PRINT_CONFIG) {
        printf("token: [%s]\n", tok.c_str());
    }
    return tok;
}

bool isFloat( string myString ) {
    std::istringstream iss(myString);
    float f;
    iss >> noskipws >> f;
    return iss.eof() && !iss.fail(); 
}

float ConfigParser::parseUnary(map<string, float> &m) {
    string v = nextToken();
    if (DEBUG_PRINT_CONFIG) {
        printf("parseUnary: v=[%s]\n", v.c_str());
    }
    if (v == "-") {
        return - parseUnary(m);
    } else if (isFloat(v)) {
        return stof(v);
    } else {
        return m[v];
    }
}
float ConfigParser::parseOp(map<string, float> &m, int precedence) {
    if (DEBUG_PRINT_CONFIG) {
        printf("parseOp-called\n");
    }
    auto prec = [](string t) {
        if (t == "") {
            return -1;
        }
        switch (t[0]) {
        case '+': return 10;
        case '-': return 10;
        case '*': return 20;
        case '/': return 20;
        case '\n': return -1;
        }
        return -2;
    };
    auto op = [](string op, float v1, float v2) {
        switch (op[0]) {
        case '+': return v1 + v2;
        case '-': return v1 - v2;
        case '*': return v1 * v2;
        case '/': return v1 / v2;
        }
        return v1;
    };
    float lvalue = parseUnary(m);
    while (true) {
        string next = peekToken();
        int nprec = prec(next);
        if (DEBUG_PRINT_CONFIG) {
            printf("parseOp-iter: left=%f, next=[%s], prec=%d\n", lvalue, next.c_str(), nprec);
        }
        if (nprec < -1) {
            error("Invalid Token!");
            return 0;
        } else if (nprec < precedence) {
            return lvalue;
        } else if (nprec >= 0) {
            expect(next);
            float rvalue = parseOp(m, nprec);
            lvalue = op(next, lvalue, rvalue);
        }
    }
}

map<string, float> ConfigParser::parseFile() {
    reset();
    map<string, float> m;
    string t;
    while ((t = nextToken()) != "") {
        line++;
        string name = t;
        expect("=");
        float v = parseOp(m, 0);
        expect("\n");
        m.insert(std::pair<string, float>(name, v));
        if (DEBUG_PRINT_CONFIG) {
            printf("Loaded var(%s)=%f from file\n", name.c_str(), v);
        }
    }

    return m;
}


void ReloadConstants() {
    auto cp = ConfigParser(PHYSICS_CONFIG_PATH);
    auto cmap = cp.parseFile();
    ACCEL = cmap["ACCEL"];
    MAX_VELOCITY = cmap["MAX_VELOCITY"];
    JUMP_VELOCITY = cmap["JUMP_VELOCITY"];
    FRICTION_FACTOR = cmap["FRICTION_FACTOR"];
    AIR_FRICTION_FACTOR = cmap["AIR_FRICTION_FACTOR"];
    GRAVITY = cmap["GRAVITY"];
    SPRINT_SPEED_MODIFIER =cmap["SPRINT_SPEED_MODIFIER"];
    JUMP_COOLDOWN =cmap["JUMP_COOLDOWN"];
    JUMP_EXTENSION_TIME = cmap["JUMP_EXTENSION_TIME"];
    CLIMB_SPEED = cmap["CLIMB_SPEED"];
}


