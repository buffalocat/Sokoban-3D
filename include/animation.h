#ifndef ANIMATION_H
#define ANIMATION_H

#include "common.h"

class Animation {
public:
    Animation();
    virtual ~Animation() = 0;
    virtual bool update() = 0;
    virtual FPoint3 dpos() = 0;
    virtual Point3 shift_pos(Point3) = 0;
};

class LinearAnimation: public Animation {
public:
    LinearAnimation(Point3);
    virtual ~LinearAnimation();
    bool update();
    FPoint3 dpos();
    Point3 shift_pos(Point3);

private:
    Point3 d_;
    int frames_;
};

#endif // ANIMATION_H
