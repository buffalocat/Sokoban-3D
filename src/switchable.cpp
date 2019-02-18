#include "switchable.h"

Switchable::Switchable(bool default_state, bool initial_state):
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

void Switchable::receive_signal(bool signal, RoomMap* room_map, DeltaFrame* delta_frame, std::vector<Block*>* fall_check) {
    if (active_ ^ waiting_ == signal) {
        return;
    }
    if (delta_frame) {
        delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_, room_map));
    }
    waiting_ = !can_set_state(default_ ^ signal, room_map);
    if (active_ != waiting_ ^ signal) {
        active_ = !active_;
        apply_state_change(room_map, fall_check);
    }
}

void Switchable::apply_state_change(RoomMap* room_map, std::vector<Block*>* fall_check) {}

void Switchable::check_waiting(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (waiting_ && can_set_state(!(default_ ^ active_), room_map)) {
        if (delta_frame) {
            delta_frame->push(std::make_unique<SwitchableDelta>(this, active_, waiting_, room_map));
        }
        waiting_ = false;
        active_ = !active_;
        apply_state_change(room_map, nullptr);
    }
}
