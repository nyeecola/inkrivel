#pragma once

#include <string>
#include "character.hpp"

using namespace std;

class Map {
public:
    Character *characterList[8];
    Model model;
    float scale;
};
