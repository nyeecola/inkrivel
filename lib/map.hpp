#pragma once

#include <string>
#include "character.hpp"
#include "udp.hpp"

using namespace std;

class Map {
public:
    Character *characterList[MAX_PLAYERS];
    Model model;
    float scale;
};
