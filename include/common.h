#ifndef COMMON_H
#define COMMON_H

#include <deque>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

const float BOARD_SIZE = 16.0f;

struct Point {
    int x;
    int y;
};

bool operator==(const Point& a, const Point& b);

struct PosHash {
    std::size_t operator()(const Point& p) const;
};

typedef std::pair<Point, unsigned int> PosId;
typedef std::vector<PosId> PosIdVec;
typedef std::unordered_map<Point, unsigned int, PosHash> PosIdMap;

enum class Layer {
    Floor,
    Player,
    Solid,
    COUNT,
};

#endif // COMMON_H
