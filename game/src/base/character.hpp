#ifndef CHARACTER_H
#define CHARACTER_H

#include "position.hpp"
#include <string>

using namespace std;

class Character {
public:
    Character(string name);
    string getName();
private:
    string name;
    Position position;
};

#endif
