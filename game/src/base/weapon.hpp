#ifndef WEAPON_H
#define WEAPON_H

#include <string>

using namespace std;

class Weapon {
public:
    Weapon(string name, int damage, float range);
    int getDamage();
    float getRange();
private:
    string name;
    int damage;
    float range;
};

#endif