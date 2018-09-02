#include "character.hpp"

Character::Character(string _name, Weapon _weapon): name(_name), weapon(_weapon) {}

string Character::getName() {
    return this->name;
}
