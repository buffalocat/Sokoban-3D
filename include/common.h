#ifndef COMMON_H
#define COMMON_H

//TODO : remove many redundant includes from common.h!!

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wshadow"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma GCC diagnostic pop

#include <memory>
#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "point.h"
#include "color_constants.h"

void clamp(int* n, int a, int b);

//NOTE: all enums here should be explicitly numbered, because they are largely
// used in serializing maps.  Maps should never go "out of date", unless a
// feature absolutely has to be removed from the system (even then, the
// numbering of the items around it should stay the same).

enum class Sticky : unsigned char {
    None = 0,
    Weak = 1,
    Strong = 2,
    AllStick = 3,
    Snake = 4,
    SnakeWeak = 5,
    All = 7,
};

Sticky operator &(Sticky a, Sticky b);

enum class RidingState {
    Free = 1,
    Bound = 2,
    Riding = 3,
};

const int GLOBAL_WALL_ID = 1;

enum class ObjCode {
    NONE = 0,
    PushBlock = 1,
    SnakeBlock = 2,
    Wall = 3,
    Player = 4,
    GateBody = 5,
};

enum class ModCode {
    NONE = 0,
    Door = 1,
    Car = 2,
    PressSwitch = 3,
    Gate = 4,
};

enum class CameraCode {
    NONE = 0,
    Free = 1,
    Fixed = 2,
    Clamped = 3,
    Null = 4,
};

struct ObjCodeHash {
    std::size_t operator()(const ObjCode& c) const;
};

struct CameraCodeHash {
    std::size_t operator()(const CameraCode& c) const;
};

enum {
    GREEN = 0,
    PINK = 1,
    PURPLE = 2,
    DARK_PURPLE = 3,
    BLUE = 4,
    RED = 5,
    DARK_RED = 6,
    BLACK = 7,
    LIGHT_GREY = 8,
    ORANGE = 9,
    YELLOW = 10,
    GREY = 11,
    DARK_GREY = 12,
};

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

// Must satisfy MAX_COOLDOWN < MOVEMENT_FRAMES for smooth motion
const int MOVEMENT_FRAMES = 8;
const int MAX_COOLDOWN = 7;

const int UNDO_COMBO_FIRST = 4;
const int UNDO_COMBO_SECOND = 10;
const int UNDO_COOLDOWN_FIRST = 7;
const int UNDO_COOLDOWN_SECOND = 4;
const int UNDO_COOLDOWN_FINAL = 2;

const int MAX_UNDO_DEPTH = 1000;

const float DEFAULT_CAM_RADIUS = 16.0;
const float DEFAULT_CAM_TILT = 0.3;
const float DEFAULT_CAM_ROTATION = 0.0;

const int FAST_MAP_MOVE = 10;

const std::string MAPS_MAIN = "maps\\main\\";
const std::string MAPS_TEMP = "maps\\temp\\";

enum class MapCode {
    Dimensions = 1, // The dimensions of the room as 1 byte integers
    FullLayer = 2, // Create a new full layer
    SparseLayer = 3, // Create a new sparse layer
    DefaultPos = 4, // Mark the position to start the player at when loading from this map (only useful for testing, or a select few rooms)
    OffsetPos = 5, // The position that (0,0) was at when the room was created
    Objects = 6, // Read in all map objects
    CameraRects = 7, // Get a camera context rectangle
    SnakeLink = 8, // Link two snakes (1 = Right, 2 = Down)
    DoorDest = 9, // Give a door a destination Map + Pos
    Signaler = 10, // List of Switches and Switchables linked to a Signaler
    Walls = 11, // List of positions of walls
    PlayerData = 12, // Like Walls, the Player is listed separately from other objects
    End = 255,
};

const std::unordered_map<int, Point3> MOVEMENT_KEYS {
    {GLFW_KEY_RIGHT, {1, 0, 0}},
    {GLFW_KEY_LEFT,  {-1,0, 0}},
    {GLFW_KEY_DOWN,  {0, 1, 0}},
    {GLFW_KEY_UP,    {0,-1, 0}},
};

const std::unordered_map<int, Point3> EDITOR_MOVEMENT_KEYS {
    {GLFW_KEY_D, {1, 0, 0}},
    {GLFW_KEY_A, {-1,0, 0}},
    {GLFW_KEY_S, {0, 1, 0}},
    {GLFW_KEY_W, {0,-1, 0}},
    {GLFW_KEY_E, {0, 0, 1}},
    {GLFW_KEY_Q, {0, 0,-1}},
};

#endif // COMMON_H
