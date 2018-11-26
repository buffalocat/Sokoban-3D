#include "colorcycle.h"
#include <iostream>

ColorCycle::ColorCycle(unsigned char color): color_ {color}, size_ {1}, index_ {0} {}

ColorCycle::ColorCycle(unsigned char* b): color_ {}, size_ {b[0]}, index_ {b[1]} {
    for (int i = 0; i < size_; ++i) {
        color_[i] = b[i+2];
    }
}

ColorCycle::~ColorCycle() {}

unsigned char ColorCycle::color() {
    return color_[index_];
}

void ColorCycle::insert_color(unsigned char color) {
    color_[size_] = index_;
    ++size_;
}

void ColorCycle::cycle(bool undo) {
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
}
