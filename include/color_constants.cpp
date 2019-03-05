#include "color_constants.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

bool Color4::operator==(Color4& c) {
    return r == c.r && g == c.g && b == c.b && a == c.a;
}

#pragma GCC diagnostic pop
