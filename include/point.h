#ifndef POINT_H
#define POINT_H

#include <ostream>

struct Point2 {
    int x;
    int y;
};

struct Point3;

Point3 operator+(const Point3& p, const Point3& q);
Point3 operator-(const Point3& p, const Point3& q);

struct Point3 {
    int x;
    int y;
    int z;
    Point3& operator+=(const Point3& p) {
        return *this = *this + p;
    }
    Point3& operator-=(const Point3& p) {
        return *this = *this - p;
    }
};

struct FPoint3 {
    float x;
    float y;
    float z;
    FPoint3(float ax, float ay, float az): x {ax}, y {ay}, z {az} {}
    FPoint3(const Point3& p): x {static_cast<float>(p.x)}, y {static_cast<float>(p.y)}, z {static_cast<float>(p.z)} {}
};

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

struct Point2Hash {
    std::size_t operator()(const Point2& p) const {
        return (p.x << 8) + p.y;
    }
};

struct Point3Hash {
    std::size_t operator()(const Point3& p) const {
        return (p.x << 16) + (p.y << 8) + p.z;
    }
};

#endif // POINT_H
