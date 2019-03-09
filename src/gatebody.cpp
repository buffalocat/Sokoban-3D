#include "gatebody.h"


#include "point.h"
#include "gate.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "moveprocessor.h"

#include "animation.h"

#include "delta.h"

GateBody::GateBody(Gate* gate, Point3 pos):
PushBlock(pos, gate->color_, gate->pushable(), gate->gravitable(), Sticky::None),
gate_ {}, gate_pos_ {}, transition_animation_ {} {
    set_gate(gate);
}

// For orphaned GateBodies
GateBody::GateBody(Point3 pos, int color, bool pushable, bool gravitable):
PushBlock(pos, color, pushable, gravitable, Sticky::None),
gate_ {}, gate_pos_ {}, transition_animation_ {} {}

GateBody::GateBody(const GateBody& other): PushBlock(other), transition_animation_ {} {}

GateBody::~GateBody() {}

std::string GateBody::name() {
    return "GateBody";
}

ObjCode GateBody::obj_code() {
    return ObjCode::GateBody;
}

// Orphaned GateBodies need to be serialized!
bool GateBody::skip_serialization() {
    return gate_ != nullptr;
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

Point3 GateBody::gate_pos() {
    return gate_pos_;
}

void GateBody::set_gate(Gate* gate) {
    gate_ = gate;
    gate_pos_ = gate->pos();
}

Point3 GateBody::update_gate_pos(DeltaFrame* delta_frame) {
    Point3 dpos = gate_->pos() - gate_pos_;
    if (!(dpos == Point3{})) {
        delta_frame->push(std::make_unique<GatePosDelta>(this, dpos));
        gate_pos_ = gate_->pos();
    }
    return dpos;
}

void GateBody::collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& to_check) {
    if (gate_) {
        to_check.push_back(gate_->parent_);
    }
}

void GateBody::set_gate_transition_animation(bool state, MoveProcessor* mp) {
    transition_animation_ = std::make_unique<GateTransitionAnimation>(state);
    if (gate_ && state) {
        if (PositionalAnimation* anim = gate_->parent_->animation_.get()) {
            animation_ = anim->duplicate();
            mp->add_to_moving_blocks(this);
        }
    }
}

// TODO: consider making this (and StateAnimation) available to all objects (if useful)
bool GateBody::update_state_animation() {
    if (transition_animation_ && transition_animation_->update()) {
        transition_animation_.reset(nullptr);
        return true;
    }
    return false;
}

void GateBody::reset_state_animation() {
    transition_animation_.reset(nullptr);
}

bool GateBody::state_animation() {
    return transition_animation_ != nullptr;
}


void GateBody::draw(GraphicsManager* gfx) {
    FPoint3 p {real_pos()};
    float height = transition_animation_ ? transition_animation_->height() : 1.0f;
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z - (1.0 - height)/2, p.y));
    model = glm::scale(model, glm::vec3(0.7f, height, 0.7f));
    gfx->set_tex(Texture::Edges);
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    gfx->draw_cube();
}

