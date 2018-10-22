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
    int health;
    Model model;
    int ammo;
    int max_ammo;
    int atk_delay; // in millis
    int starting_atk_delay; // in millis

    bool dead;
    float respawn_timer;
};

class Projectile {
public:
    Vector pos;
    Vector dir;
    float radius;
    float speed;
    uint8_t team;
    uint8_t damage;
};
