#include "signaler.h"

Signaler::Signaler(unsigned char threshold, bool persistent, bool active):
count_ {0}, threshold_ {threshold},
active_ {active}, persistent_ {persistent},
switches_ {}, switchables_ {} {}

Signaler::~Signaler() {}

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

void Signaler::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame, std::vector<Block*>* fall_check) {
    if (!(active_ && persistent_) && ((count_ >= threshold_) != active_)) {
        if (delta_frame) {
            delta_frame->push(std::make_unique<SignalerToggleDelta>(this));
        }
        active_ = !active_;
        for (Switchable* obj : switchables_) {
            obj->receive_signal(active_, room_map, delta_frame, fall_check);
        }
    }
}

void Signaler::serialize(MapFileO& file) {
    file << MapCode::Signaler;
    file << threshold_;
    file << (persistent_ | (active_ << 1));
    file << switches_.size();
    file << switchables_.size();
    for (auto& obj : switches_) {
        file << obj->pos();
    }
    for (auto& obj : switchables_) {
        file << obj->pos();
    }
}

bool Signaler::remove_object(GameObject* obj) {
    switchables_.erase(std::remove(switchables_.begin(), switchables_.end(), obj), switchables_.end());
    switches_.erase(std::remove(switches_.begin(), switches_.end(), obj), switches_.end());
    return switchables_.empty() || switches_.empty();
}
