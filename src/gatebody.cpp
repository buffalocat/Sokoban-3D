#include "gatebody.h"

#include "common.h"
#include "point.h"
#include "gate.h"
#include "mapfile.h"
#include "graphicsmanager.h"

GateBody::GateBody(Gate* parent): PushBlock(parent->pos() + Point3{0,0,1}, parent->color_, parent->pushable(), parent->gravitable(), Sticky::None), parent_ {parent} {}

// For orphaned GateBodies
GateBody::GateBody(Point3 pos, int color, bool pushable, bool gravitable):
PushBlock(pos, color, pushable, gravitable, Sticky::None), parent_ {} {}

GateBody::~GateBody() {}

std::string GateBody::name() {
    return "GateBody";
}

ObjCode GateBody::obj_code() {
    return ObjCode::GateBody;
}

// Orphaned GateBodies need to be serialized!
bool GateBody::skip_serialization() {
    return parent_ != nullptr;
}

void GateBody::serialize(MapFileO& file) {
    file << color_ << pushable_ << gravitable_;
}

std::unique_ptr<GameObject> GateBody::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[3];
    file.read(b, 3);
    return std::make_unique<GateBody>(pos, b[0], b[1], b[2]);
}

void GateBody::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& to_check) {
    if (parent_) {
        to_check.push_back(parent_->parent_);
    }
}

void GateBody::draw(GraphicsManager* gfx) {
    FPoint3 p {real_pos()};
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y));
    model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
    gfx->set_tex(Texture::Edges);
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    gfx->draw_cube();
}

