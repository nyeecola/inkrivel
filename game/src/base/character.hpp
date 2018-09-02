#ifndef CHARACTER_H
#define CHARACTER_H

#include "position.hpp"
#include "weapon.hpp"
#include <string>

using namespace std;

class Character {
public:
    Character(string name, Weapon weapon);
    string getName();
private:
    string name;
    Weapon weapon;
    Position position;
};

#endif
