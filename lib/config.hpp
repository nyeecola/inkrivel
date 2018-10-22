#pragma once

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define AVAILABLE_CHARACTERS 4
#define DEBUG 0
#define ERROR -1
#define ever ;;
#define MAX_PLAYERS 8

// TODO: maybe do something about this
#define MAX_PROJECTILES 200
#define MAX_PAINT_POINTS 70

#define MAX(a, b) ((a) > (b) ? a : b)
#define MIN(a, b) ((a) < (b) ? a : b)

#define SWIM_GOOD_FACTOR 2
#define SWIM_BAD_FACTOR 0.5

#define AMMO_BOX_X_OFFSET 0.3
#define AMMO_BOX_Y_OFFSET 0.1
#define AMMO_BOX_WIDTH 0.2
#define AMMO_BOX_HEIGHT 0.4
#define AMMO_BOX_BORDER 0.01

#define STARTING_AMMO 100
#define ATK_DELAY 35

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
#define TIMER_DURATION_IN_SECONDS 190
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

#define LIGHT_START_X (-7)
#define LIGHT_START_Y (-5)
#define LIGHT_END_X (7)
#define LIGHT_END_Y (5)

#define LOGIN_SERVER_IP "127.0.0.1"
#define LOGIN_SERVER_PORT ":3000"

#define SERVER_ADDRESS LOGIN_SERVER_IP
#define SERVER_PORT 27222
#define CHAT_SERVER_PORT 17555

typedef enum {
    ROLO = 0,
    SNIPER,
    ASSAULT,
    BUCKET,
    TEST,
} CharacterId;
