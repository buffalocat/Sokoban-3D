#ifndef COMMON_H
#define COMMON_H

#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class GameObject;
class Block;

const float BOARD_SIZE = 16.0f;

struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b);

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

#endif // COMMON_H
