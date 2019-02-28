#include "colorcycle.h"

ColorCycle::ColorCycle(): color_ {0}, size_ {1}, index_ {0} {}

ColorCycle::ColorCycle(int color): color_ {color}, size_ {1}, index_ {0} {}

ColorCycle::ColorCycle(unsigned char* b): color_ {}, size_ {b[0]}, index_ {b[1]} {
    for (int i = 0; i < size_; ++i) {
        color_[i] = b[i+2];
    }
}

ColorCycle::~ColorCycle() {}

int ColorCycle::color() {
    return color_[index_];
}

void ColorCycle::set_current(int color) {
    color_[index_] = color;
}

bool ColorCycle::cycle(bool undo) {
    if (size_ == 1) {
        return false;
    }
    if (!undo) {
        ++index_;
        if (index_ == size_) {
            index_ = 0;
        }
    } else {
        if (!index_) {
            index_ = size_ - 1;
        } else {
            --index_;
        }
    }
    return true;
}
