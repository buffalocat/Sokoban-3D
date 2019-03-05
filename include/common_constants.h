#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include "point.h"

const int GLOBAL_WALL_ID = 1;

const int MAX_COLOR_CYCLE = 5;

// NOTE: the order matters here, for serialization reasons!
const Point3 DIRECTIONS[6] = {{-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0}, {0,0,1}, {0,0,-1}};
const Point3 H_DIRECTIONS[4] = {{-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0}};

const int MAX_ROOM_DIMS = 255;

#define SOKOBAN_LARGE_WINDOW
#ifdef SOKOBAN_LARGE_WINDOW

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 900;
#else
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
#endif
const int MESH_SIZE = 50;

constexpr float ORTHO_WIDTH = (float)SCREEN_WIDTH/(float)MESH_SIZE;
constexpr float ORTHO_HEIGHT = (float)SCREEN_HEIGHT/(float)MESH_SIZE;

const int DEFAULT_BOARD_WIDTH = 17;
const int DEFAULT_BOARD_HEIGHT = 13;


// Must satisfy MAX_COOLDOWN < HORIZONTAL_MOVEMENT_FRAMES for smooth motion
const int HORIZONTAL_MOVEMENT_FRAMES = 8;
const int FALL_MOVEMENT_FRAMES = 4;
const int MAX_COOLDOWN = 7;

const int UNDO_COMBO_FIRST = 4;
const int UNDO_COMBO_SECOND = 10;
const int UNDO_COOLDOWN_FIRST = 7;
const int UNDO_COOLDOWN_SECOND = 4;
const int UNDO_COOLDOWN_FINAL = 2;

const int MAX_UNDO_DEPTH = 1000;

const int FAST_MAP_MOVE = 10;

#endif // COMMON_CONSTANTS_H
