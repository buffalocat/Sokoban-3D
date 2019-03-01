#include "gatebody.h"

#include "common.h"
#include "point.h"
#include "gate.h"
#include "mapfile.h"
#include "graphicsmanager.h"

GateBody::GateBody(Gate* parent): PushBlock(parent->pos() + Point3{0,0,1}, parent->color_, parent->pushable(), parent->gravitable(), Sticky::None), parent_ {parent} {}

GateBody::~GateBody() {}

ObjCode GateBody::obj_code() {
    return ObjCode::GateBody;
}

bool GateBody::skip_serialization() {
    return true;
}

void GateBody::draw(GraphicsManager* gfx) {
    Point3 p = pos_;
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y));
    model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
    gfx->set_tex(Texture::Edges);
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    gfx->draw_cube();
}
