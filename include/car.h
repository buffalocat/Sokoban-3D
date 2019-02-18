#ifndef CAR_H
#define CAR_H

#include "objectmodifier.h"
#include "colorcycle.h"

class Car: public ObjectModifier {
public:
    Car(GameObject* parent, ColorCycle color_cycle);
    virtual ~Car();

    unsigned char color();
    void insert_color(unsigned char color);
    bool cycle_color(bool undo);

private:
    ColorCycle color_cycle_;
};

#endif // CAR_H
