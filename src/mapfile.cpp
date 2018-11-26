#include "mapfile.h"
#include "block.h"

MapFileI::MapFileI(std::string path): file_ {} {
    file_.open(path, std::ios::in | std::ios::binary);
}

MapFileI::~MapFileI() {
    file_.close();
}

void MapFileI::read(unsigned char* b, int n) {
    file_.read((char *)b, n);
}

void MapFileI::read_str(char* b, int n) {
    file_.read(b, n);
}

float MapFileI::read_float() {
    unsigned char b[2];
    file_.read((char *)b, 2);
    return (float)b[0] + (float)b[1]/256.0;
}

Point3 MapFileI::read_point3() {
    unsigned char b[3];
    file_.read((char *)b, 3);
    return {b[0], b[1], b[2]};
}

FPoint3 MapFileI::read_fpoint3() {
    float x = read_float();
    float y = read_float();
    float z = read_float();
    return {x,y,z};
}

MapFileO::MapFileO(std::string path): file_ {} {
    file_.open(path, std::ios::out | std::ios::binary);
}

MapFileO::~MapFileO() {
    file_.close();
}

void MapFileO::operator<<(Point3 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) pos.y;
    file_ << (unsigned char) pos.z;
}

void MapFileO::operator<<(FPoint3 pos) {
    *this << pos.x;
    *this << pos.y;
    *this << pos.z;
}

void MapFileO::operator<<(unsigned char n) {
    file_ << n;
}

void MapFileO::operator<<(int n) {
    file_ << (unsigned char) n;
}

void MapFileO::operator<<(unsigned int n) {
    file_ << (unsigned char) n;
}

void MapFileO::operator<<(float f) {
    file_ << (unsigned char)f;
    file_ << (unsigned char)(256.0*f);
}

void MapFileO::operator<<(bool b) {
    file_ << (unsigned char) b;
}

void MapFileO::operator<<(std::string str) {
    file_ << (unsigned char) str.size();
    file_.write(str.c_str(), str.size());
}

void MapFileO::operator<<(MapCode code) {
    file_ << (unsigned char) code;
}

void MapFileO::operator<<(ObjCode code) {
    file_ << (unsigned char) code;
}

void MapFileO::operator<<(CameraCode code) {
    file_ << (unsigned char) code;
}

void MapFileO::operator<<(RidingState state) {
    file_ << (unsigned char) state;
}

void MapFileO::operator<<(ColorCycle& color) {
    file_ << color.size_;
    file_ << color.index_;
    for (int i = 0; i < color.size_; ++i) {
        file_ << color.color_[i];
    }
}
