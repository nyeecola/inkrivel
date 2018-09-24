#pragma once

#include <string>
#include "character.hpp"
#include "../../../lib/udp.hpp"

using namespace std;

class Map {
public:
    Character *characterList[MAX_PLAYERS];
    Model model;
};
