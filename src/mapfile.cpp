#include "mapfile.h"

#include "colorcycle.h"

MapFileI::MapFileI(std::string path): file_ {} {
    file_.open(path, std::ios::in | std::ios::binary);
}

MapFileI::~MapFileI() {
    file_.close();
}


void MapFileI::read(unsigned char* b, int n) {
    file_.read((char *)b, n);
}

unsigned char MapFileI::read_byte() {
    unsigned char b;
    file_.read((char *)&b, 1);
    return b;
}

Point3 MapFileI::read_point3() {
    unsigned char b[3];
    read(b, 3);
    return {b[0], b[1], b[2]};
}

std::string MapFileI::read_str() {
    unsigned char n;
    char b[256] = "";
    read(&n, 1);
    file_.read(b, n);
    return std::string(b, n);
}

MapFileI& operator>>(MapFileI& f, int& v) {
    unsigned char b;
    f.read(&b, 1);
    v = b;
    return f;
}

MapFileI& operator>>(MapFileI& f, float& v) {
    unsigned char b[2];
    f.read(b, 2);
    v = (float)b[0] + (float)b[1]/256.0;
    return f;
}

MapFileI& operator>>(MapFileI& f, Point2& v) {
    unsigned char b[2];
    f.read(b, 2);
    v = {b[0], b[1]};
    return f;
}

MapFileI& operator>>(MapFileI& f, Point3_S16& v) {
    unsigned char b[6];
    f.read(b, 6);
    v.x = b[0] + (b[1] << 8) - (1 << 15);
    v.y = b[2] + (b[3] << 8) - (1 << 15);
    v.z = b[4] + (b[5] << 8) - (1 << 15);
    return f;
}

MapFileI& operator>>(MapFileI& f, Point3& v) {
    unsigned char b[3];
    f.read(b, 3);
    v = {b[0], b[1], b[2]};
    return f;
}

MapFileI& operator>>(MapFileI& f, FPoint3& v) {
    f >> v.x >> v.y >> v.z;
    return f;
}

MapFileI& operator>>(MapFileI& f, ColorCycle& v) {
    unsigned char b[2 + MAX_COLOR_CYCLE];
    f.read(b, 2);
    f.read(b+2, b[0]);
    v = ColorCycle{b};
    return f;
}


MapFileO::MapFileO(std::string path): file_ {} {
    file_.open(path, std::ios::out | std::ios::binary);
}

MapFileO::~MapFileO() {
    file_.close();
}

MapFileO& MapFileO::operator<<(unsigned char n) {
    file_ << n;
    return *this;
}

MapFileO& MapFileO::operator<<(int n) {
    file_ << (unsigned char) n;
    return *this;
}

MapFileO& MapFileO::operator<<(unsigned int n) {
    file_ << (unsigned char) n;
    return *this;
}

MapFileO& MapFileO::operator<<(float f) {
    file_ << (unsigned char)f;
    file_ << (unsigned char)(256.0*f);
    return *this;
}

MapFileO& MapFileO::operator<<(bool b) {
    file_ << (unsigned char) b;
    return *this;
}

MapFileO& MapFileO::operator<<(Point2 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) pos.y;
    return *this;
}

MapFileO& MapFileO::operator<<(Point3_S16 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) ((pos.x + (1 << 15)) >> 8);
    file_ << (unsigned char) pos.y;
    file_ << (unsigned char) ((pos.y + (1 << 15)) >> 8);
    file_ << (unsigned char) pos.z;
    file_ << (unsigned char) ((pos.z + (1 << 15)) >> 8);
    return *this;
}

// NOTE: all Point3's that get serialized are actually in the range [0,255]
MapFileO& MapFileO::operator<<(Point3 pos) {
    file_ << (unsigned char) pos.x;
    file_ << (unsigned char) pos.y;
    file_ << (unsigned char) pos.z;
    return *this;
}

MapFileO& MapFileO::operator<<(FPoint3 pos) {
    *this << pos.x;
    *this << pos.y;
    *this << pos.z;
    return *this;
}

MapFileO& MapFileO::operator<<(std::string str) {
    file_ << (unsigned char) str.size();
    file_.write(str.c_str(), str.size());
    return *this;
}

// TODO: consider replacing these with a template!

MapFileO& MapFileO::operator<<(MapCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(ObjCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(ModCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(CameraCode code) {
    file_ << (unsigned char) code;
    return *this;
}

MapFileO& MapFileO::operator<<(Sticky sticky) {
    file_ << (unsigned char) sticky;
    return *this;
}

MapFileO& MapFileO::operator<<(RidingState state) {
    file_ << (unsigned char) state;
    return *this;
}

MapFileO& MapFileO::operator<<(ColorCycle& color) {
    file_ << (unsigned char) color.size_;
    file_ << (unsigned char) color.index_;
    for (int i = 0; i < color.size_; ++i) {
        file_ << (unsigned char) color.color_[i];
    }
    return *this;
}
