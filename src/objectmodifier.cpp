#include "objectmodifier.h"

#include "gameobject.h"

ObjectModifier::ObjectModifier(GameObject* parent): parent_ {parent} {}

ObjectModifier::~ObjectModifier() {}

Point3 ObjectModifier::pos() {
    return parent_->pos_;
}

Point3 ObjectModifier::shifted_pos(Point3 d) {
    return parent_->pos_ + d;
}

Point3 ObjectModifier::pos_above() {
    return parent_->pos_ + Point3{0,0,1};
}

int ObjectModifier::color() {
    return parent_->color_;
}

bool ObjectModifier::pushable() {
    return parent_->pushable_;
}

bool ObjectModifier::gravitable() {
    return parent_->gravitable_;
}

void ObjectModifier::setup_on_put(RoomMap* room_map) {}

void ObjectModifier::cleanup_on_take(RoomMap* room_map) {}

void ObjectModifier::cleanup_on_destruction(RoomMap* room_map) {}

void ObjectModifier::setup_on_undestruction(RoomMap* room_map) {}

void ObjectModifier::map_callback(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void ObjectModifier::collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&) {}
