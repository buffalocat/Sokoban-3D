#ifndef MAPFILE_H
#define MAPFILE_H

#include <fstream>

#include "common.h"
#include "colorcycle.h"

namespace Deser {
    float f(unsigned char* b);
    Point3 p3(unsigned char* b);
    FPoint3 fp3(unsigned char* b);
}

class MapFileI {
public:
    MapFileI(std::string path);
    virtual ~MapFileI();
    void read(unsigned char* b, int n);

    Point3 read_point3();
    std::string read_str();
    ColorCycle read_color_cycle();

private:
    std::ifstream file_;
};

class MapFileO {
public:
    MapFileO(std::string path);
    virtual ~MapFileO();
    void operator<<(Point3);
    void operator<<(FPoint3);
    void operator<<(unsigned char);
    void operator<<(std::string);
    void operator<<(ColorCycle&);
    void operator<<(MapCode);
    void operator<<(ObjCode);
    void operator<<(CameraCode);
    void operator<<(RidingState);
    void operator<<(int);
    void operator<<(unsigned int);
    void operator<<(bool);
    void operator<<(float);

private:
    std::ofstream file_;
};

#endif // MAPFILE_H
