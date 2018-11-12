#include "door.h"

#include "graphicsmanager.h"

MapLocation::MapLocation(Point p, std::string name): pos {p}, map_name {name} {}

Door::Door(int x, int y): GameObject(x, y), dest_ {} {}

Door::~Door() {}

ObjCode Door::obj_code() {
    return ObjCode::Door;
}

Layer Door::layer() {
    return Layer::Floor;
}

void Door::set_dest(Point pos, std::string name) {
    dest_ = std::make_unique<MapLocation>(pos,name);
}

MapLocation* Door::dest() {
    return dest_.get();
}

// Door serializes trivially
GameObject* Door::deserialize(unsigned char* b) {
    return new Door(b[0], b[1]);
}

bool Door::relation_check() {
    return dest_ != nullptr;
}

void Door::relation_serialize(std::ofstream& file) {
    file << (unsigned char)MapCode::DoorDest;
    file << (unsigned char)pos_.x;
    file << (unsigned char)pos_.y;
    file << (unsigned char)dest_->pos.x;
    file << (unsigned char)dest_->pos.y;
    unsigned char n = dest_->map_name.size();
    file << n;
    file.write(dest_->map_name.c_str(), n);
}

void Door::draw(GraphicsManager* gfx) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0, p.y));
    model = glm::scale(model, glm::vec3(1, 0.1, 1));
    gfx->set_model(model);
    if (dest_) {
        gfx->set_color(COLORS[BLUE]);
    } else {
        gfx->set_color(COLORS[DARK_RED]);
    }
    gfx->draw_cube();
}
