#ifndef COMMON_H
#define COMMON_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#include <glm/glm.hpp>

#pragma GCC diagnostic pop

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>

class GameObject;
class Block;

struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b);

std::ostream& operator<<(std::ostream& os, const Point& p);

struct PointHash {
    std::size_t operator()(const Point& p) const;
};

typedef std::unordered_set<Point, PointHash> PointSet;
typedef std::vector<Point> PointVec;
typedef std::vector<GameObject*> ObjVec;
typedef std::unordered_set<GameObject*> ObjSet;
typedef std::unordered_set<Block*> BlockSet;
typedef std::unordered_map<Point, GameObject*, PointHash> PosIdMap;
//typedef std::pair<Point, GameObject*> PosId;
//typedef std::vector<PosId> PosIdVec;

enum class Layer {
    Floor,
    Player,
    Solid,
    COUNT,
};

enum class ObjCode {
    NONE,
    Wall,
    PushBlock,
    SnakeBlock,
};

struct ObjCodeHash {
    std::size_t operator()(const ObjCode& c) const;
};

const glm::vec4 GREEN = glm::vec4(0.6f, 0.9f, 0.7f, 1.0f);
const glm::vec4 PINK = glm::vec4(0.9f, 0.6f, 0.7f, 1.0f);
const glm::vec4 PURPLE = glm::vec4(0.7f, 0.5f, 0.9f, 1.0f);
const glm::vec4 DARK_PURPLE = glm::vec4(0.3f, 0.2f, 0.6f, 1.0f);
const glm::vec4 RED = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
const glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 ORANGE = glm::vec4(1.0f, 0.7f, 0.3f, 1.0f);

// NOTE: the order matters here, for serialization reasons!
const Point DIRECTIONS[4] = {Point{-1,0}, Point{0,-1}, Point{1,0}, Point{0,1}};


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int MESH_SIZE = 50;

const float ORTHO_WIDTH = (float)SCREEN_WIDTH/(float)MESH_SIZE;
const float ORTHO_HEIGHT = (float)SCREEN_HEIGHT/(float)MESH_SIZE;

const int DEFAULT_BOARD_WIDTH = 17;
const int DEFAULT_BOARD_HEIGHT = 13;

const int MAX_COOLDOWN = 5;

const float DEFAULT_CAM_RADIUS = 16.0;


const bool DEV_MODE = true;

#endif // COMMON_H
