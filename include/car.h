#ifndef CAR_H
#define CAR_H

#include "common.h"
#include "objectmodifier.h"
#include "colorcycle.h"
#include "mapfile.h"

class Car: public ObjectModifier {
public:
    Car(GameObject* parent, ColorCycle color_cycle);
    virtual ~Car();

    ModCode mod_code();
    void serialize(MapFileO& file);
    static std::unique_ptr<ObjectModifier> deserialize(GameObject*, MapFileI& file);

    void insert_color(unsigned char color);
    bool cycle_color(bool undo);

private:
    ColorCycle color_cycle_;
};

#endif // CAR_H
