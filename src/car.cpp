#include "car.h"

Car::Car(GameObject* parent, ColorCycle color_cycle): ObjectModifier(parent), color_cycle_ {color_cycle} {}

Car::~Car() {}

unsigned char Car::color() {
    return color_cycle_.color();
}

void Car::insert_color(unsigned char color) {
    color_cycle_.insert_color(color);
}

bool Car::cycle_color(bool undo) {
    return color_cycle_.cycle(undo);
}
