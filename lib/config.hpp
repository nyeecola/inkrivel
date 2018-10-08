#pragma once

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define AVAILABLE_CHARACTERS 4
#define DEBUG 0
#define ERROR -1
#define ever ;;
#define MAX_PLAYERS 8
// TODO: maybe do something about this
#define MAX_PROJECTILES 100
#define MAX_PAINT_POINTS 100

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

typedef enum {
    TEST,
    ROLO,
    SNIPER,
    ASSAULT,
    BUCKET,
} CharacterId;
