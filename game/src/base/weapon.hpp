#ifndef WEAPON_H
#define WEAPON_H

#include <string>

using namespace std;

class Weapon {
public:
    Weapon(string name, int damage, float range);
    
    string name;
    int damage;
    float range;
};

#endif
