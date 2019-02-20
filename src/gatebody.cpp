#include "gatebody.h"

#include "common.h"
#include "point.h"
#include "gate.h"
#include "mapfile.h"
#include "graphicsmanager.h"

GateBody::GateBody(Gate* base): PushBlock(base->pos() + Point3{0,0,1}, base->color(), base->pushable(), base->gravitable(), Sticky::None), base_ {base} {}

GateBody::~GateBody() {}

ObjCode GateBody::obj_code() {
    return ObjCode::GateBody;
}

void GateBody::draw(GraphicsManager* gfx) {
    Point3 p = pos_;
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y));
    model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
    gfx->set_model(model);
    gfx->set_color(COLORS[LIGHT_GREY]);
    gfx->draw_cube();
}
