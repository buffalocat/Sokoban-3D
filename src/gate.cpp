#include "gate.h"

#include "point.h"
#include <memory>
#include "gatebody.h"
#include "mapfile.h"
#include "roommap.h"

#include "moveprocessor.h"
#include "graphicsmanager.h"

// Gates should be initialized down in case they are "covered" at load time
Gate::Gate(GameObject* parent, GateBody* body, int color, bool def, bool active, bool waiting):
Switchable(parent, def, active, waiting), color_ {color}, body_ {body} {}

Gate::~Gate() {}

std::string Gate::name() {
    return "Gate";
}

ModCode Gate::mod_code() {
    return ModCode::Gate;
}

void Gate::serialize(MapFileO& file) {
    bool body_alive = (body_ != nullptr);
    file << color_ << default_ << active_ << waiting_ << body_alive;
    if (body_alive) {
        file << Point3_S16{body_->pos_};
    }
}

void Gate::deserialize(MapFileI& file, RoomMap* room_map, GameObject* parent) {
    unsigned char b[5];
    file.read(b, 5);
    auto gate = std::make_unique<Gate>(parent, nullptr, b[0], b[1], b[2], b[3]);
    // Is the body alive?
    if (b[4]) {
        Point3_S16 body_pos {};
        file >> body_pos;
        auto gate_body_unique = std::make_unique<GateBody>(gate.get(), Point3{body_pos});
        gate->body_ = gate_body_unique.get();
        room_map->create_abstract(std::move(gate_body_unique), nullptr);
    }
    parent->set_modifier(std::move(gate));
}

void Gate::collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& to_check) {
    if (body_ && state()) {
        to_check.push_back(body_);
    }
}

bool Gate::can_set_state(bool state, RoomMap* room_map) {
    return body_ && (!state || (room_map->view(body_->pos_) == nullptr));
}

void Gate::apply_state_change(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (body_) {
        if (state()) {
            room_map->put_loud(body_, delta_frame);
        } else {
            room_map->take_loud(body_, delta_frame);
        }
        GameObject* above = room_map->view(pos() + Point3{0,0,2});
        if (above && above->gravitable_) {
            mp->add_to_fall_check(above);
        }
    }
}

void Gate::map_callback(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (body_) {
        Point3 dpos = body_->update_gate_pos(delta_frame);
        if (!state()) {
            body_->abstract_shift(dpos, delta_frame);
        }
    }
    check_waiting(room_map, delta_frame, mp);
}

// TODO: make sure Gate listeners are handled correctly in all cases!!
void Gate::setup_on_put(RoomMap* room_map) {
    if (body_) {
        room_map->add_listener(this, body_->pos_);
        room_map->activate_listener_of(this);
    }
}

void Gate::cleanup_on_take(RoomMap* room_map) {
    if (body_) {
        room_map->remove_listener(this, body_->pos_);
    }
}

void Gate::draw(GraphicsManager* gfx, FPoint3 p) {
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z + 0.5f, p.y));
    model = glm::scale(model, glm::vec3(0.8f, 0.1f, 0.8f));
    gfx->set_tex(Texture::Blank);
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    gfx->draw_cube();
}

std::unique_ptr<ObjectModifier> Gate::duplicate(GameObject* parent, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto dup = std::make_unique<Gate>(*this);
    dup->parent_ = parent;
    if (body_) {
        auto body_dup = std::make_unique<GateBody>(*body_);
        body_dup->set_gate(dup.get());
        dup->body_ = body_dup.get();
        if (state()) {
            room_map->create(std::move(body_dup), delta_frame);
        } else {
            room_map->create_abstract(std::move(body_dup), delta_frame);
        }
    }
    dup->connect_to_signalers();
    return std::move(dup);
}
