#pragma once

#include "render-types.hpp"
#include "vector.hpp"
#include <string>

using namespace std;

class Character {
public:
    //string name;
    Vector pos;
    float speed;
    Vector dir;
    Quat rotation;
    //int damage;
    //float range;
    float hit_radius;
    Model model;
};

class Projectile {
public:
    Vector pos;
    Vector dir;
    float radius;
    float speed;
    uint8_t team;
};
