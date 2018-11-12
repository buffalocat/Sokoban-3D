#include "switch.h"
#include "roommap.h"
#include "delta.h"
#include "graphicsmanager.h"

Switchable::Switchable(int x, int y, bool default_state, bool initial_state):
GameObject(x, y),
default_ {default_state},
active_ {(bool)(default_state ^ initial_state)},
waiting_ {(bool)(default_state ^ initial_state)} {}

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
Gate::Gate(int x, int y, bool def): Switchable(x, y, def, false) {}

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

void Gate::draw(GraphicsManager* gfx) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, .9f * state() - 0.45f, p.y));
    model = glm::scale(model, glm::vec3(0.7f, 1.0f, 0.7f));
    gfx->set_model(model);
    gfx->set_color(COLORS[DARK_PURPLE]);
    gfx->draw_cube();
}

Signaler::Signaler(unsigned int threshold, bool persistent, bool active):
count_ {0}, threshold_ {threshold},
active_ {active}, persistent_ {persistent},
switches_ {}, switchables_ {} {}

void Signaler::push_switchable(Switchable* obj) {
    switchables_.push_back(obj);
}

void Signaler::push_switch(Switch* obj) {
    switches_.push_back(obj);
    obj->push_signaler(this);
}

void Signaler::receive_signal(bool signal) {
    if (signal) {
        ++count_;
    } else {
        --count_;
    }
}

void Signaler::toggle() {
    active_ = !active_;
}

void Signaler::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (!(active_ && persistent_) && ((count_ >= threshold_) != active_)) {
        delta_frame->push(std::make_unique<SignalerToggleDelta>(this));
        active_ = !active_;
        for (Switchable* obj : switchables_) {
            obj->receive_signal(active_, room_map, delta_frame);
        }
    }
}

void Signaler::serialize(std::ofstream& file) {
    file << (unsigned char)threshold_;
    file << (unsigned char)(persistent_ | (active_ << 1));
    file << (unsigned char)switches_.size();
    file << (unsigned char)switchables_.size();
    for (auto& obj : switches_) {
        Point pos = obj->pos();
        file << (unsigned char)pos.x;
        file << (unsigned char)pos.y;
        file << (unsigned char)obj->obj_code();
    }
    for (auto& obj : switchables_) {
        Point pos = obj->pos();
        file << (unsigned char)pos.x;
        file << (unsigned char)pos.y;
        file << (unsigned char)obj->obj_code();
    }
}

Switch::Switch(int x, int y, bool persistent, bool active): GameObject(x, y),
persistent_ {persistent}, active_ {active}, signalers_ {} {}

Switch::~Switch() {}

void Switch::push_signaler(Signaler* signaler) {
    signalers_.push_back(signaler);
}

void Switch::toggle() {
    active_ = !active_;
    for (Signaler* signaler : signalers_) {
        signaler->receive_signal(active_);
    }
}

PressSwitch::PressSwitch(int x, int y, unsigned char color, bool persistent, bool active):
Switch(x, y, persistent, active), color_ {color} {}

ObjCode PressSwitch::obj_code() {
    return ObjCode::PressSwitch;
}

Layer PressSwitch::layer() {
    return Layer::Floor;
}

void PressSwitch::serialize(std::ofstream& file) {
    file << color_;
    file << (unsigned char)(persistent_ | (active_ << 1));
}

GameObject* PressSwitch::deserialize(unsigned char* b) {
    return new PressSwitch(b[0], b[1], b[2], b[3] & 1, b[3] & 2);
}

void PressSwitch::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame, std::unordered_set<Signaler*>& check) {
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

bool PressSwitch::should_toggle(RoomMap* room_map) {
    return active_ ^ (room_map->view(pos_, Layer::Solid) != nullptr);
}

void PressSwitch::draw(GraphicsManager* gfx) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, -0.4f, p.y));
    model = glm::scale(model, glm::vec3(0.9f, 1.0f, 0.9f));
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    if (persistent_) {
        if (active_) {
            gfx->set_tex(glm::vec2(2,1));
        } else {
            gfx->set_tex(glm::vec2(1,1));
        }
    } else {
        gfx->set_tex(glm::vec2(0,1));
    }
    gfx->draw_cube();
    gfx->set_tex(glm::vec2(0,0));
}
