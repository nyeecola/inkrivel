#pragma once

#include "../../../lib/render-types.hpp"
#include "../../../lib/vector.hpp"
#include <string>

using namespace std;

class Character {
public:
    //string name;
    Vector pos;
    float speed;
    Vector dir;
    //int damage;
    //float range;
    float hit_radius;
    Model model;
};
