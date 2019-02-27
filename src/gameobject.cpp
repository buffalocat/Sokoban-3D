#include "common.h"

#include "gameobject.h"

#include "roommap.h"
#include "graphicsmanager.h"
#include "delta.h"
#include "mapfile.h"
#include "objectmodifier.h"
#include "animation.h"

#include "moveprocessor.h"

// id_ begins in an "inconsistent" state - it *must* be set by the GameObjectArray
GameObject::GameObject(Point3 pos, unsigned char color, bool pushable, bool gravitable):
    modifier_ {}, animation_ {}, comp_ {},
    pos_ {pos}, id_ {-1},
    color_ {color}, pushable_ {pushable}, gravitable_ {gravitable} {}

GameObject::~GameObject() {}

bool GameObject::relation_check() {
    return false;
}

void GameObject::relation_serialize(MapFileO& file) {}

Point3 GameObject::shifted_pos(Point3 d) {
    return pos_ + d;
}

void GameObject::setup_on_put(RoomMap*) {}

void GameObject::cleanup_on_take(RoomMap*) {}

void GameObject::cleanup_on_destruction(RoomMap*) {}

void GameObject::setup_on_undestruction(RoomMap*) {}

void GameObject::set_modifier(std::unique_ptr<ObjectModifier> mod) {
    modifier_ = std::move(mod);
}

ObjectModifier* GameObject::modifier() {
    return modifier_.get();
}

// NOTE: these can be static_casts as long as the code using them is careful
PushComponent* GameObject::push_comp() {
    return dynamic_cast<PushComponent*>(comp_);
}

FallComponent* GameObject::fall_comp() {
    return dynamic_cast<FallComponent*>(comp_);
}


void GameObject::collect_sticky_component(RoomMap* room_map, Sticky sticky_level, Component* comp) {
    std::vector<GameObject*> to_check {this};
    while (!to_check.empty()) {
        GameObject* cur = to_check.back();
        to_check.pop_back();
        if (cur->comp_) {
            continue;
        }
        cur->comp_ = comp;
        comp->blocks_.push_back(cur);
        cur->collect_sticky_links(room_map, sticky_level, to_check);
        cur->collect_special_links(room_map, sticky_level, to_check);
        if (modifier_) {
            modifier_->collect_sticky_links(room_map, sticky_level, to_check);
        }
    }
}

bool GameObject::has_sticky_neighbor(RoomMap* room_map) {
    for (Point3 d : H_DIRECTIONS) {
        if (GameObject* adj = room_map->view(pos_ + d)) {
            if ((adj->color_ == color_) && static_cast<bool>(adj->sticky() & sticky())) {
                return true;
            }
        }
    }
    return false;
}

void GameObject::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>&) {}

void GameObject::reset_animation() {
    animation_.reset(nullptr);
}

void GameObject::set_linear_animation(Point3 d) {
    animation_ = std::make_unique<LinearAnimation>(d);
}

void GameObject::update_animation() {
    if (animation_ && animation_->update()) {
        animation_.reset(nullptr);
    }
}

void GameObject::shift_pos_from_animation() {
    pos_ = animation_->shift_pos(pos_);
}

#include <iostream>

FPoint3 GameObject::real_pos() {
    if (animation_) {
        return pos_ + animation_->dpos();
    } else {
        return pos_;
    }
}
