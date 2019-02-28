#include "switch.h"

#include "roommap.h"
#include "signaler.h"

#include <algorithm>


Switch::Switch(GameObject* parent, bool persistent, bool active): ObjectModifier(parent),
persistent_ {persistent}, active_ {active}, signalers_ {} {}

Switch::~Switch() {}

void Switch::push_signaler(Signaler* signaler) {
    signalers_.push_back(signaler);
}

void Switch::connect_to_signalers() {
    for (Signaler* s : signalers_) {
        s->push_switch(this);
    }
}

void Switch::toggle() {
    active_ = !active_;
    for (auto& signaler : signalers_) {
        signaler->receive_signal(active_);
    }
}

void Switch::cleanup_on_destruction(RoomMap* room_map) {
    for (Signaler* s : signalers_) {
        s->remove_object(this);
    }
}

void Switch::setup_on_undestruction(RoomMap* room_map) {
    connect_to_signalers();
}
