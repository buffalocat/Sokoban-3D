#include "switchable.h"

#include "delta.h"
#include "moveprocessor.h"
#include "signaler.h"

Switchable::Switchable(GameObject* parent, bool def, bool active, bool waiting): ObjectModifier(parent),
default_ {def},
active_ {active},
waiting_ {waiting},
signalers_ {} {}

Switchable::~Switchable() {}

void Switchable::push_signaler(Signaler* signaler) {
    signalers_.push_back(signaler);
}

void Switchable::connect_to_signalers() {
    for (Signaler* s : signalers_) {
        s->push_switchable(this);
    }
}

bool Switchable::state() {
    return default_ ^ active_;
}

void Switchable::receive_signal(bool signal, RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (active_ ^ waiting_ == signal) {
        return;
    }
    delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_));
    waiting_ = !can_set_state(default_ ^ signal, room_map);
    if (active_ != waiting_ ^ signal) {
        active_ = !active_;
        apply_state_change(room_map, delta_frame, mp);
    }
}

void Switchable::apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*) {}

void Switchable::check_waiting(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (waiting_ && can_set_state(!(default_ ^ active_), room_map)) {
        delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_));
        waiting_ = false;
        active_ = !active_;
        apply_state_change(room_map, delta_frame, mp);
    }
}


void Switchable::cleanup_on_destruction(RoomMap* room_map) {
    for (Signaler* s : signalers_) {
        s->remove_object(this);
    }
}

void Switchable::setup_on_undestruction(RoomMap* room_map) {
    connect_to_signalers();
}
