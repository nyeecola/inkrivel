#include "weapon.hpp"

Weapon::Weapon(string _name, int _damage, float _range): name(_name), range(_range) {
    if (_damage <= 0) {
        this->damage = 1;
    } else {
        this->damage = _damage;
    }
}

int Weapon::getDamage() {
    return this->damage;
}