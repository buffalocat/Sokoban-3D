#include "door.h"

#include "mapfile.h"
#include "gameobject.h"
#include "graphicsmanager.h"

MapLocation::MapLocation(Point3 p, std::string room_name): pos {p}, name {room_name} {}

Door::Door(GameObject* parent, bool def): ObjectModifier(parent), Switchable(def, def), dest_ {} {}

Door::~Door() {}

ObjCode Door::obj_code() {
    return ObjCode::Door;
}

void Door::set_dest(Point3 pos, std::string name) {
    dest_ = std::make_unique<MapLocation>(pos,name);
}

MapLocation* Door::dest() {
    return dest_.get();
}

void Door::serialize(MapFileO& file) {
    file << default_;
}

/*
GameObject* Door::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[1];
    file.read(b,1);
    return new Door(pos, b[0]);
}
*/

bool Door::relation_check() {
    return dest_ != nullptr;
}

void Door::relation_serialize(MapFileO& file) {
    file << MapCode::DoorDest;
    file << parent_->pos_;
    file << dest_->pos;
    file << dest_->name;
}

bool Door::can_set_state(bool state, RoomMap* room_map) {
    return true;
}

void Door::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y));
    model = glm::scale(model, glm::vec3(1, 0.1, 1));
    gfx->set_model(model);
    if (dest_ && state()) {
        gfx->set_color(COLORS[BLUE]);
    } else {
        gfx->set_color(COLORS[DARK_RED]);
    }
    gfx->draw_cube();
}
