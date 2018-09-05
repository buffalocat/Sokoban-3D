#ifndef COMMON_H
#define COMMON_H

#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class GameObject;

const float BOARD_SIZE = 16.0f;

struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b);

struct PosHash {
    std::size_t operator()(const Point& p) const;
};

typedef std::pair<Point, GameObject*> PosId;
typedef std::vector<PosId> PosIdVec;
typedef std::unordered_map<Point, GameObject*, PosHash> PosIdMap;
typedef std::unordered_set<GameObject*> ObjSet;

enum class Layer {
    Floor,
    Player,
    Solid,
    COUNT,
};

#endif // COMMON_H
