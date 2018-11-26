#include "common.h"

#include "gameobject.h"
#include "block.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "delta.h"
#include "mapfile.h"


GameObject::GameObject(Point3 pos): pos_ {pos} {}

GameObject::~GameObject() {}

void GameObject::serialize(MapFileO& file) {}

bool GameObject::relation_check() {
    return false;
}

void GameObject::relation_serialize(MapFileO& file) {}

Point3 GameObject::pos() {
    return pos_;
}

// This could maybe (?) be optimized by instead taking a Direction enum, for the 6 directions
Point3 GameObject::shifted_pos(Point3 d) {
    return {pos_.x + d.x, pos_.y + d.y, pos_.z + d.z};
}

void GameObject::set_pos(Point3 p) {
    pos_ = p;
}

void GameObject::set_pos_auto(Point3 p, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto self_unique = room_map->take_quiet(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_, room_map));
    }
    pos_ = p;
    room_map->put_quiet(std::move(self_unique));
}

void GameObject::shift_pos(Point3 d) {
    pos_ += d;
}

void GameObject::shift_pos_auto(Point3 d, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto self_unique = room_map->take_quiet(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_, room_map));
    }
    pos_ += d;
    room_map->put_quiet(std::move(self_unique));
}


Wall::Wall(Point3 pos): GameObject(pos) {}

Wall::~Wall() {}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

void Wall::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(COLORS[BLACK]);
    gfx->draw_cube();
}

GameObject* Wall::deserialize(MapFileI& file) {
    return new Wall(file.read_point3());
}
