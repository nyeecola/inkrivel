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
    float ammo;
    int atk_delay; // in millis
    int starting_atk_delay; // in millis
    bool swimming;

    bool dead;
    float respawn_timer;

    // these variable are bad solutions to real problems
    // TODO: clean this up
    Vector normal_sum;
    Vector paint_max_z;
    int paint_face;

    // these variable is only for assault
    bool alternate_fire_assault;
};

class Projectile {
public:
    Vector pos;
    Vector velocity;
    float radius;
    uint8_t team;
    uint8_t damage;

    int character_id;
};
