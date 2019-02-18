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
GameObject::GameObject(Point3 pos, int color, bool pushable, bool gravitable):
modifier_ {}, animation_ {}, pos_ {pos}, id_ {-1},
color_ {color}, pushable_ {pushable}, gravitable_ {gravitable} {}

GameObject::~GameObject() {}

void GameObject::serialize(MapFileO& file) const {}

bool GameObject::relation_check() const {
    return false;
}

void GameObject::relation_serialize(MapFileO& file) const {}

Point3 GameObject::pos() const {
    return pos_;
}

Point2 GameObject::posh() const {
    return {pos_.x, pos_.y};
}

int GameObject::z() const {
    return pos_.z;
}

Point3 GameObject::shifted_pos(Point3 d) const {
    return pos_ + d;
}

void GameObject::setup_on_put(RoomMap*) {}

void GameObject::cleanup_on_take(RoomMap*) {}

void GameObject::cleanup_on_destruction(RoomMap*) {}

void GameObject::setup_on_undestruction(RoomMap*) {}

ObjectModifier* GameObject::modifier() {
    return modifier_.get();
}


void GameObject::collect_strong_component(RoomMap* room_map, PushComponent* comp, Point3 dir,
        std::unordered_map<GameObject*, PushComponent*>& push_comps) {
    std::vector<GameObject*> to_check {this};
    while (!to_check.empty()) {
        GameObject* cur = to_check.back();
        to_check.pop_back();
        if (push_comps.count(cur)) {
            continue;
        }
        push_comps[cur] = comp;
        comp->blocks_.push_back(cur);
        cur->collect_sticky_links(room_map, Sticky::Strong, to_check);
        if (modifier_) {
            modifier_->collect_sticky_links(room_map, Sticky::Strong, to_check);
        }
    }
}

void GameObject::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>&) const {}

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

FPoint3 GameObject::real_pos() {
    if (animation_) {
        return pos_ + animation_->dpos();
    } else {
        return pos_;
    }
}
