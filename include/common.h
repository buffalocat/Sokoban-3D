#ifndef COMMON_H
#define COMMON_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma GCC diagnostic pop

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <memory>

#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class GameObject;
class Block;

struct Point {
    int x;
    int y;
};

typedef Point Point2;

struct Point3 {
    int x;
    int y;
    int z;
    Point h() {return {x, y};}
    Point3& operator+=(const Point3&);
};

Point3 operator+(const Point3& p, const Point3& q);

struct FPoint3 {
    float x;
    float y;
    float z;
    FPoint3(float, float, float);
    FPoint3(const Point3&);
};

bool operator==(const Point& a, const Point& b);

Point3 operator*(const int, const Point3&);

std::ostream& operator<<(std::ostream& os, const Point& p);

std::ostream& operator<<(std::ostream& os, const Point3& p);

struct PointHash {
    std::size_t operator()(const Point& p) const;
};

void clamp(int* n, int a, int b);

//NOTE: all enums here should be explicitly numbered, because they are largely
// used in serializing maps.  Maps should never go "out of date", unless a
// feature absolutely has to be removed from the system (even then, the
// numbering of the items around it should stay the same).

enum class RidingState {
    Free = 1,
    Bound = 2,
    Riding = 3,
};

enum class ObjCode {
    NONE = 0,
    Wall = 1,
    NonStickBlock = 2,
    WeakBlock = 3,
    StickyBlock = 4,
    SnakeBlock = 5,
    Door = 6,
    Player = 7,
    PressSwitch = 8,
    Gate = 9,
    GateBody = 10,
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

const glm::vec4 COLORS[] = {
    glm::vec4(0.6f, 0.9f, 0.7f, 1.0f),
    glm::vec4(0.9f, 0.6f, 0.7f, 1.0f),
    glm::vec4(0.7f, 0.5f, 0.9f, 1.0f),
    glm::vec4(0.3f, 0.2f, 0.6f, 1.0f),
    glm::vec4(0.0f, 0.3f, 0.8f, 1.0f),
    glm::vec4(1.0f, 0.5f, 0.5f, 1.0f),
    glm::vec4(0.6f, 0.0f, 0.1f, 1.0f),
    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
    glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
    glm::vec4(1.0f, 0.7f, 0.3f, 1.0f),
    glm::vec4(0.7f, 0.7f, 0.3f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
};

const int NUM_GREYS = 8;

const glm::vec4 GREYS[NUM_GREYS] = {
    //glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
    glm::vec4(0.2f, 0.2f, 0.2f, 1.0f),
    glm::vec4(0.3f, 0.3f, 0.3f, 1.0f),
    glm::vec4(0.4f, 0.4f, 0.4f, 1.0f),
    glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
    glm::vec4(0.6f, 0.6f, 0.6f, 1.0f),
    glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
    glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
    glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
};

// NOTE: the order matters here, for serialization reasons!
const Point3 DIRECTIONS[6] = {{-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0}, {0,0,1}, {0,0,-1}};
const Point3 H_DIRECTIONS[6] = {{-1,0,0}, {0,-1,0}, {1,0,0}, {0,1,0}};

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

const int MAX_COOLDOWN = 5;

const int MAX_UNDO_DEPTH = 1000;

const float DEFAULT_CAM_RADIUS = 16.0;
const float DEFAULT_CAM_TILT = 0.3;
const float DEFAULT_CAM_ROTATION = 0.0;

const int FAST_MAP_MOVE = 10;

const std::string MAPS_MAIN = "maps\\main\\";
const std::string MAPS_TEMP = "maps\\temp\\";

enum class MapCode {
    Dimensions = 1, // Gets width and height as 1 byte integers
    DefaultPos = 2, // Mark the position to start the player at when loading from this map (only useful for testing, or a select few rooms)
    Objects = 3, // Read in all map objects
    CameraRect = 4, // Get a camera context rectangle
    SnakeLink = 5, // Link two snakes (1 = Right, 2 = Down)
    DoorDest = 6, // Give a door a destination Map + Pos
    Signaler = 7, // List of Switches and Switchables linked to a Signaler
    FullLayer = 8, // Create a new full layer
    SparseLayer = 9, // Create a new sparse layer
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
