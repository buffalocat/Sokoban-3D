#include "point.h"


Point3& Point3::operator+=(const Point3& p) {
    return *this = *this + p;
}

Point3& Point3::operator-=(const Point3& p) {
    return *this = *this - p;
}

Point3 operator+(const Point3& p, const Point3& q) {
    return {p.x + q.x, p.y + q.y, p.z + q.z};
}

Point3 operator-(const Point3& p, const Point3& q) {
    return {p.x - q.x, p.y - q.y, p.z - q.z};
}

Point3 operator-(const Point3& p) {
    return {-p.x, -p.y, -p.z};
}

FPoint3 operator+(const Point3& p, const FPoint3& q) {
    return {p.x + q.x, p.y + q.y, p.z + q.z};
}

bool operator==(const Point2& a, const Point2& b) {
    return a.x == b.x && a.y == b.y;
}

bool operator==(const Point3& a, const Point3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

Point3 operator*(const int a, const Point3& p) {
    return {a*p.x, a*p.y, a*p.z};
}

FPoint3 operator*(const float a, const FPoint3& p) {
    return {a*p.x, a*p.y, a*p.z};
}

std::ostream& operator<<(std::ostream& os, const Point2& p) {
    return os << "(" <<  p.x << "," << p.y << ")";
}

std::ostream& operator<<(std::ostream& os, const Point3& p) {
    return os << "(" <<  p.x << "," << p.y << "," << p.z << ")";
}

std::size_t Point2Hash::operator()(const Point2& p) const {
    return (p.x << 8) + p.y;
}

std::size_t Point3Hash::operator()(const Point3& p) const {
    return (p.x << 16) + (p.y << 8) + p.z;
}
