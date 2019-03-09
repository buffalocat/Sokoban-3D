#ifndef ANIMATION_H
#define ANIMATION_H

#include <memory>
#include "point.h"

class Animation {
public:
    Animation();
    virtual ~Animation() = 0;
    bool update();

protected:
    int frames_;
};

class PositionalAnimation: public Animation {
public:
    virtual FPoint3 dpos() = 0;
    virtual Point3 shift_pos(Point3) = 0;

    virtual std::unique_ptr<PositionalAnimation> duplicate() = 0;
};

class LinearAnimation: public PositionalAnimation {
public:
    LinearAnimation(Point3);
    ~LinearAnimation();
    FPoint3 dpos();
    Point3 shift_pos(Point3);

    std::unique_ptr<PositionalAnimation> duplicate();

private:
    Point3 d_;
};

// TODO (maybe): make an intermediate StateAnimation class

class GateTransitionAnimation: public Animation {
public:
    GateTransitionAnimation(bool state);
    ~GateTransitionAnimation();
    float height();

private:
    bool state_;
};

#endif // ANIMATION_H
