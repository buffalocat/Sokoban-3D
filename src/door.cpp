#include "door.h"

#include "mapfile.h"
#include "gameobject.h"
#include "graphicsmanager.h"

MapLocation::MapLocation(Point3 p, std::string room_name): pos {p}, name {room_name} {}

Door::Door(GameObject* parent, bool def, bool active): Switchable(parent, def, active), dest_ {} {}

Door::~Door() {}

Door::Door(const Door& d): Switchable(d.parent_, d.default_, d.active_), dest_ {} {}

ModCode Door::mod_code() {
    return ModCode::Door;
}

void Door::set_dest(Point3 pos, std::string name) {
    dest_ = std::make_unique<MapLocation>(pos,name);
}

MapLocation* Door::dest() {
    return dest_.get();
}

void Door::serialize(MapFileO& file) {
    file << default_ << active_;
}

void Door::deserialize(MapFileI& file, RoomMap*, GameObject* parent) {
    unsigned char b[2];
    file.read(b, 2);
    parent->set_modifier(std::make_unique<Door>(parent, b[0], b[1]));
}

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

void Door::draw(GraphicsManager* gfx, FPoint3 p) {
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
