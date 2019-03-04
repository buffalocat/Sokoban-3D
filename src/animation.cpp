#include "animation.h"

Animation::Animation() {}

Animation::~Animation() {}


LinearAnimation::LinearAnimation(Point3 d): Animation(), d_ {d}, frames_ {HORIZONTAL_MOVEMENT_FRAMES - 1} {}

LinearAnimation::~LinearAnimation() {

}

bool LinearAnimation::update() {
    --frames_;
    return frames_ == 0;
}

FPoint3 LinearAnimation::dpos() {
    return (-(float)frames_/(float)HORIZONTAL_MOVEMENT_FRAMES)*FPoint3{d_};
}

Point3 LinearAnimation::shift_pos(Point3 p) {
    return p + d_;
}
