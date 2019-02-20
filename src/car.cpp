#include "car.h"

#include "gameobject.h"

Car::Car(GameObject* parent, ColorCycle color_cycle): ObjectModifier(parent), color_cycle_ {color_cycle} {}

Car::~Car() {}

ModCode Car::mod_code() {
    return ModCode::Car;
}

void Car::serialize(MapFileO& file) {
    file << color_cycle_;
}

std::unique_ptr<ObjectModifier> Car::deserialize(GameObject* parent, MapFileI& file) {
    ColorCycle color_cycle;
    file >> color_cycle;
    return std::make_unique<Car>(parent, color_cycle);
}

void Car::insert_color(unsigned char color) {
    color_cycle_.insert_color(color);
}

bool Car::cycle_color(bool undo) {
    bool result = color_cycle_.cycle(undo);
    parent_->color_ = color_cycle_.color();
    return result;
}
