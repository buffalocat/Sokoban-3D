#include "multicolorblock.h"

TwoColorBlock::TwoColorBlock(unsigned char alt_color): alt_color_ {alt_color} {}

TwoColorBlock::~TwoColorBlock() {}


TwoColorPushBlock::TwoColorPushBlock(int x, int y, unsigned char color, unsigned char alt_color, bool car, StickyLevel sticky):
PushBlock(x, y, color, car, sticky), TwoColorBlock(alt_color) {}

TwoColorPushBlock::~TwoColorPushBlock() {}

ObjCode TwoColorPushBlock::obj_code() {
    return ObjCode::TwoColorPushBlock;
}

void TwoColorPushBlock::swap_colors() {
    unsigned char temp = alt_color_;
    alt_color_ = color_;
    color_ = temp;
}

void TwoColorPushBlock::serialize(std::ofstream& file) {
    PushBlock::serialize(file);
    file << alt_color_;
}

GameObject* TwoColorPushBlock::deserialize(unsigned char* b) {
    bool is_car = b[3] >> 7;
    StickyLevel sticky = static_cast<StickyLevel>(b[3] & 3);
    return new TwoColorPushBlock(b[0], b[1], b[2], b[4], is_car, sticky);
}
