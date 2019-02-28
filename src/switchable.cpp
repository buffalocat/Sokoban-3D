#include "switchable.h"

#include "delta.h"
#include "moveprocessor.h"

Switchable::Switchable(GameObject* parent, bool default_state, bool initial_state): ObjectModifier(parent),
default_ {default_state},
active_ {(bool)(default_state ^ initial_state)},
waiting_ {(bool)(default_state ^ initial_state)} {}

Switchable::~Switchable() {}

void Switchable::set_aw(bool active, bool waiting, RoomMap* room_map) {
    if (active_ != active) {
        active_ = active;
        apply_state_change(room_map, nullptr);
    }
    waiting_ = waiting;
}

bool Switchable::state() {
    return default_ ^ active_;
}

void Switchable::receive_signal(bool signal, RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (active_ ^ waiting_ == signal) {
        return;
    }
    if (delta_frame) {
        delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_, room_map));
    }
    waiting_ = !can_set_state(default_ ^ signal, room_map);
    if (active_ != waiting_ ^ signal) {
        active_ = !active_;
        apply_state_change(room_map, mp);
    }
}

void Switchable::apply_state_change(RoomMap* room_map, MoveProcessor* mp) {}

void Switchable::check_waiting(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (waiting_ && can_set_state(!(default_ ^ active_), room_map)) {
        if (delta_frame) {
            delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_, room_map));
        }
        waiting_ = false;
        active_ = !active_;
        apply_state_change(room_map, mp);
    }
}
