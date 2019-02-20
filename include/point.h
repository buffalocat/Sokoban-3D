#ifndef POINT_H
#define POINT_H

#include <ostream>

// Standard Points serialize their components as unsigned chars
struct Point2 {
    int x;
    int y;
};

struct Point2_S16 {
    int x;
    int y;
};

struct Point3 {
    int x;
    int y;
    int z;
    Point3& operator+=(const Point3& p);
    Point3& operator-=(const Point3& p);
    Point2 h() {return {x,y};}
};

struct FPoint3 {
    float x;
    float y;
    float z;
    FPoint3(): x {}, y {}, z {} {}
    FPoint3(float ax, float ay, float az): x {ax}, y {ay}, z {az} {}
    FPoint3(const Point3& p): x {static_cast<float>(p.x)}, y {static_cast<float>(p.y)}, z {static_cast<float>(p.z)} {}
};

Point3 operator+(const Point3& p, const Point3& q);
Point3 operator-(const Point3& p, const Point3& q);

Point3 operator-(const Point3& p);

FPoint3 operator+(const Point3& p, const FPoint3& q);

bool operator==(const Point2& a, const Point2& b);

bool operator==(const Point3& a, const Point3& b);

Point3 operator*(const int a, const Point3& p);

FPoint3 operator*(const float a, const FPoint3& p);

std::ostream& operator<<(std::ostream& os, const Point2& p);

std::ostream& operator<<(std::ostream& os, const Point3& p);

struct Point2Hash {
    std::size_t operator()(const Point2& p) const;
};

struct Point3Hash {
    std::size_t operator()(const Point3& p) const;
};

#endif // POINT_H
