#include "objectmodifier.h"

#include "gameobject.h"

ObjectModifier::ObjectModifier(GameObject* parent): parent_ {parent} {}

ObjectModifier::~ObjectModifier() {}

Point3 ObjectModifier::pos() const {
    return parent_->pos_;
}

Point3 ObjectModifier::shifted_pos(Point3 d) const {
    return parent_->pos_ + d;
}

Point3 ObjectModifier::pos_above() const {
    return parent_->pos_ + Point3{0,0,1};
}

int ObjectModifier::color() const {
    return parent_->color_;
}

bool ObjectModifier::pushable() const {
    return parent_->pushable_;
}

bool ObjectModifier::gravitable() const {
    return parent_->gravitable_;
}

void ObjectModifier::collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&) const {}
