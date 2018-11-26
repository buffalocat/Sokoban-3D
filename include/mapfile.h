#ifndef MAPFILE_H
#define MAPFILE_H

#include "common.h"

class ColorCycle;

class MapFileI {
public:
    MapFileI(std::string path);
    virtual ~MapFileI();
    void read(unsigned char* b, int n);
    void read_str(char* b, int n);
    float read_float();
    Point3 read_point3();
    FPoint3 read_fpoint3();

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
