#include "common.h"

bool operator==(const Point& a, const Point& b)
{
    return a.x == b.x && a.y == b.y;
}

std::ostream& operator<<(std::ostream& os, const Point& p)
{
    os << "(" <<  p.x << "," << p.y << ")";
    return os;
}

std::size_t PointHash::operator()(const Point& p) const {
    return (p.x << 8) + p.y;
}

std::size_t ObjCodeHash::operator()(const ObjCode& c) const {
    return static_cast<unsigned char>(c);
}

std::size_t CameraCodeHash::operator()(const CameraCode& c) const {
    return static_cast<unsigned char>(c);
}
