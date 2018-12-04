#include "common.h"

void clamp(int* n, int a, int b) {
    *n = std::max(a, std::min(b, *n));
}

bool operator==(const Point& a, const Point& b) {
    return a.x == b.x && a.y == b.y;
}

Point3 operator+(const Point3& p, const Point3& q) {
    return {p.x + q.x, p.y + q.y, p.z + q.z};
}

Point3& Point3::operator+=(const Point3& p) {
    this->x += p.x;
    this->y += p.y;
    this->z += p.z;
    return *this;
}

Point3& Point3::operator-=(const Point3& p) {
    this->x -= p.x;
    this->y -= p.y;
    this->z -= p.z;
    return *this;
}

Point3 operator*(const int a, const Point3& p) {
    return Point3 {a*p.x, a*p.y, a*p.z};
}

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" <<  p.x << "," << p.y << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Point3& p) {
    os << "(" <<  p.x << "," << p.y << "," << p.z << ")";
    return os;
}

std::size_t PointHash::operator()(const Point& p) const {
    return (p.x << 8) + p.y;
}

FPoint3::FPoint3(float ax, float ay, float az): x {ax}, y {ay}, z {az} {}

FPoint3::FPoint3(const Point3& p): x {static_cast<float>(p.x)}, y {static_cast<float>(p.y)}, z {static_cast<float>(p.z)} {}


std::size_t ObjCodeHash::operator()(const ObjCode& c) const {
    return static_cast<unsigned char>(c);
}

std::size_t CameraCodeHash::operator()(const CameraCode& c) const {
    return static_cast<unsigned char>(c);
}
