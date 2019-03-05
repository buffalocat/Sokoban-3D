#ifndef MAPFILE_H
#define MAPFILE_H

#include <fstream>
#include <string>

#include "point.h"

class ColorCycle;

enum class MapCode;
enum class ObjCode;
enum class ModCode;
enum class CameraCode;
enum class Sticky;
enum class RidingState;

class MapFileI {
public:
    MapFileI(std::string path);
    ~MapFileI();
    void read(unsigned char* b, int n);

    unsigned char read_byte();
    Point3 read_point3();
    std::string read_str();

private:
    std::ifstream file_;
};

MapFileI& operator>>(MapFileI& f, int& v);
MapFileI& operator>>(MapFileI& f, float& v);

MapFileI& operator>>(MapFileI& f, Point2& v);
MapFileI& operator>>(MapFileI& f, Point3& v);
MapFileI& operator>>(MapFileI& f, Point3_S16& v);
MapFileI& operator>>(MapFileI& f, FPoint3& v);

MapFileI& operator>>(MapFileI& f, ColorCycle& v);



class MapFileO {
public:
    MapFileO(std::string path);
    ~MapFileO();

    MapFileO& operator<<(unsigned char);
    MapFileO& operator<<(int);
    MapFileO& operator<<(unsigned int);
    MapFileO& operator<<(bool);
    MapFileO& operator<<(float);

    MapFileO& operator<<(Point2);
    MapFileO& operator<<(Point3);
    MapFileO& operator<<(Point3_S16);
    MapFileO& operator<<(FPoint3);

    MapFileO& operator<<(std::string);
    MapFileO& operator<<(ColorCycle&);

    MapFileO& operator<<(MapCode);
    MapFileO& operator<<(ObjCode);
    MapFileO& operator<<(ModCode);
    MapFileO& operator<<(CameraCode);
    MapFileO& operator<<(Sticky);
    MapFileO& operator<<(RidingState);

private:
    std::ofstream file_;
};

#endif // MAPFILE_H
