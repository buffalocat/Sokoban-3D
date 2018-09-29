#include "switch.h"
#include "roommap.h"
#include "delta.h"
#include "shader.h"

Switchable::Switchable(bool default_state, bool initial_state): default_ {default_state},
active_ {(bool)(default_state ^ initial_state)}, waiting_ {(bool)(default_state ^ initial_state)} {}

Switchable::~Switchable() {}

void Switchable::set_aw(bool active, bool waiting) {
    active_ = active;
    waiting_ = waiting;
}

bool Switchable::state() {
    return default_ ^ active_;
}

void Switchable::receive_signal(bool signal, RoomMap* room_map, DeltaFrame* delta_frame) {
    if (active_ ^ waiting_ == signal) {
        return;
    }
    if (delta_frame) {
        delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_));
    }
    waiting_ = !can_set_state(default_ ^ signal, room_map);
    active_ = waiting_ ^ signal;
}

void Switchable::check_waiting(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (waiting_ && can_set_state(!(default_ ^ active_), room_map)) {
        if (delta_frame) {
            delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_));
        }
        waiting_ = false;
        active_ = !active_;
    }
}

// Gates should be initialized down in case they are "covered" at load time
Gate::Gate(int x, int y, bool def): GameObject(x, y), Switchable(def, false) {}

Gate::~Gate() {}

ObjCode Gate::obj_code() {
    return ObjCode::Gate;
}

Layer Gate::layer() {
    return state() ? Layer::Solid : Layer::Floor;
}

void Gate::serialize(std::ofstream& file) {
    file << (unsigned char)default_;
}

GameObject* Gate::deserialize(unsigned char* b) {
    return new Gate(b[0], b[1], (bool)b[2]);
}

bool Gate::can_set_state(bool state, RoomMap* room_map) {
    // You can always set state to false, but setting it to true requires there be
    // nothing above the gate
    return !state || (room_map->view(pos_, Layer::Solid) == nullptr);
}

void Gate::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, .9f * state() - 0.45f, p.y));
    model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
    shader->setMat4("model", model);
    shader->setVec4("color", COLORS[DARK_PURPLE]);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

Signaler::Signaler(unsigned int threshold, bool persistent): count_ {0}, threshold_ {threshold},
state_ {false}, persistent_ {persistent}, switchables_ {} {}

void Signaler::push_switchable(Switchable* obj) {
    switchables_.push_back(obj);
}

void Signaler::receive_signal(bool signal) {
    if (signal) {
        ++count_;
    } else {
        --count_;
    }
}

void Signaler::toggle() {
    state_ = !state_;
}

void Signaler::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (!(state_ && persistent_) && ((count_ >= threshold_) != state_)) {
        delta_frame->push(std::make_unique<SignalerToggleDelta>(this));
        state_ = !state_;
        for (Switchable* obj : switchables_) {
            obj->receive_signal(state_, room_map, delta_frame);
        }
    }
}

Switch::Switch(int x, int y, unsigned char color, bool persistent): GameObject(x, y),
persistent_ {persistent}, active_ {false}, color_ {color}, signalers_ {} {}

ObjCode Switch::obj_code() {
    return ObjCode::Switch;
}

Layer Switch::layer() {
    return Layer::Floor;
}

void Switch::serialize(std::ofstream& file) {
    file << color_;
    file << (unsigned char)persistent_;
}

GameObject* Switch::deserialize(unsigned char* b) {
    return new Switch(b[0], b[1], b[2], (bool)b[3]);
}

void Switch::push_signaler(Signaler* signaler) {
    signalers_.push_back(signaler);
}

bool Switch::should_toggle(RoomMap* room_map) {
    return active_ ^ (room_map->view(pos_, Layer::Solid) != nullptr);
}

void Switch::toggle() {
    active_ = !active_;
    for (Signaler* signaler : signalers_) {
        signaler->receive_signal(active_);
    }
}

void Switch::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame, std::unordered_set<Signaler*>& check) {
    if (active_ && persistent_) {
        return;
    }
    if (should_toggle(room_map)) {
        delta_frame->push(std::make_unique<SwitchToggleDelta>(this));
        toggle();
    }
    for (Signaler* signaler : signalers_) {
        check.insert(signaler);
    }
}

void Switch::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, -0.4f, p.y));
    model = glm::scale(model, glm::vec3(0.9f, 1.0f, 0.9f));
    shader->setMat4("model", model);
    shader->setVec4("color", COLORS[color_]);
    if (persistent_) {
        if (active_) {
            shader->setVec2("TexOffset", glm::vec2(2,1));
        } else {
            shader->setVec2("TexOffset", glm::vec2(1,1));
        }
    } else {
        shader->setVec2("TexOffset", glm::vec2(0,1));
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    shader->setVec2("TexOffset", glm::vec2(0,0));
}
