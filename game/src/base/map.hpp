#ifndef MAP_H
#define MAP_H

#include <string>
#include "character.hpp"

using namespace std;

class Map {
public:
    Map(int width, int height, string name);
private:
    string name;
    int width;
    int height;
    Character characterList[];
};

#endif
