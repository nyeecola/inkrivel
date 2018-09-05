#pragma once

#include "../render-types.hpp"
#include "sphere.hpp"
#include "vector.hpp"
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

    Model model;
    float hit_radius;
};
