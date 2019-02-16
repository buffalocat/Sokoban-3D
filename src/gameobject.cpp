#include "common.h"

#include <limits>

#include "gameobject.h"
#include "block.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "delta.h"
#include "mapfile.h"

// id_ begins in an "inconsistent" state - it *must* be set by the GameObjectArray
GameObject::GameObject(Point3 pos): pos_ {pos}, id_ {-1} {}

GameObject::~GameObject() {}

void GameObject::serialize(MapFileO& file) {}

bool GameObject::relation_check() {
    return false;
}

void GameObject::relation_serialize(MapFileO& file) {}

Point3 GameObject::pos() {
    return pos_;
}

Point2 GameObject::posh() {
    return {pos_.x, pos_.y};
}

int GameObject::z() {
    return pos_.z;
}

// This could maybe (?) be optimized by instead taking a Direction enum, for the 6 directions
Point3 GameObject::shifted_pos(Point3 d) {
    return {pos_.x + d.x, pos_.y + d.y, pos_.z + d.z};
}

void GameObject::set_pos(Point3 p) {
    pos_ = p;
}

void GameObject::shift_pos(Point3 d) {
    pos_ += d;
}

void GameObject::setup_on_put(RoomMap*) {}

void GameObject::cleanup_on_take(RoomMap*) {}

void GameObject::cleanup_on_destruction() {}

void GameObject::setup_on_undestruction() {}

void GameObject::pushable() {
    return false;
}

void GameObject::gravitable() {
    return false;
}

// Wall is a special GameObject, only instantiated once
// As it is completely static, and has no "identity", its pos is fake
Wall::Wall(): GameObject({INT_MAX, INT_MAX, INT_MAX}) {}

Wall::~Wall() {}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

void Wall::draw(GraphicsManager* gfx, Point3 p) {
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(GREYS[p.z % NUM_GREYS]);
    gfx->draw_cube();
}

GameObject* Wall::deserialize(MapFileI& file) {
    return new Wall();
}
