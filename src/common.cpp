#include "common.h"

bool operator==(const Point& a, const Point& b)
{
    return a.x == b.x && a.y == b.y;
}

std::size_t PointHash::operator()(const Point& p) const
{
    return (p.x << 8) + p.y;
}
