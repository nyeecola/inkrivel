#pragma once

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define AVAILABLE_CHARACTERS 4
#define DEBUG 0
#define ERROR -1
#define ever ;;
#define MAX_PLAYERS 8
// TODO: maybe do something about this
#define MAX_PROJECTILES 130
#define MAX_PAINT_POINTS 30

#define MAX(a, b) ((a) > (b) ? a : b)
#define MIN(a, b) ((a) < (b) ? a : b)

#define TEST_SCALE 0.2, 0.2, 0.2
#define ROLO_SCALE 0.2, 0.2, 0.2
#define SNIPER_SCALE 0.2, 0.2, 0.2
#define ASSAULT_SCALE 0.2, 0.2, 0.2
#define BUCKET_SCALE 0.2, 0.2, 0.2

#define MAP_SCALE 0.4

#define MAX_OBJ_VERTICES 100000
#define MAX_OBJ_FACES 100000

#define GRAVITY 0.005

#define TIMER_X SCREEN_WIDTH/2 - 42
#define TIMER_Y SCREEN_HEIGHT - 30
#define TIMER_DURATION_IN_SECONDS 30
#define RESPAWN_TIMER_X SCREEN_WIDTH/2 - 12
#define RESPAWN_TIMER_Y SCREEN_HEIGHT - 60
#define RESPAWN_DELAY 6

#define TEST_HITBOX_Z_OFFSET 0.0f
#define ROLO_HITBOX_Z_OFFSET 0.1f
#define SNIPER_HITBOX_Z_OFFSET 0.1f
#define ASSAULT_HITBOX_Z_OFFSET 0.1f
#define BUCKET_HITBOX_Z_OFFSET 0.1f

#define TEST_PROJECTILE_DAMAGE 10
#define ROLO_PROJECTILE_DAMAGE 10
#define SNIPER_PROJECTILE_DAMAGE 10
#define ASSAULT_PROJECTILE_DAMAGE 10
#define BUCKET_PROJECTILE_DAMAGE 10

#define STARTING_HEALTH 100

#define LIGHT_START_X (-15)
#define LIGHT_START_Y (-10)
#define LIGHT_END_X (15)
#define LIGHT_END_Y (10)

typedef enum {
    TEST,
    ROLO,
    SNIPER,
    ASSAULT,
    BUCKET,
} CharacterId;
