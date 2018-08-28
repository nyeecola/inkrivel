#include "character.hpp"

Character::Character(string _name): name(_name) {}

string Character::getName() {
    return this->name;
}
