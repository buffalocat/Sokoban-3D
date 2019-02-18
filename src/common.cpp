#include "common.h"

void clamp(int* n, int a, int b) {
    *n = std::max(a, std::min(b, *n));
}

std::size_t ObjCodeHash::operator()(const ObjCode& c) const {
    return static_cast<unsigned char>(c);
}

std::size_t CameraCodeHash::operator()(const CameraCode& c) const {
    return static_cast<unsigned char>(c);
}
