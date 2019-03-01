#include "signaler.h"

#include <algorithm>

#include "switch.h"
#include "switchable.h"
#include "delta.h"
#include "mapfile.h"

Signaler::Signaler(std::string label, int count, int threshold, bool persistent, bool active):
switches_ {}, switchables_ {},
label_ {label},
count_ {count}, threshold_ {threshold},
active_ {active}, persistent_ {persistent} {}

Signaler::~Signaler() {}

void Signaler::push_switchable(Switchable* obj) {
    switchables_.push_back(obj);
}

void Signaler::push_switch(Switch* obj) {
    switches_.push_back(obj);
}

void Signaler::push_switchable_mutual(Switchable* obj) {
    switchables_.push_back(obj);
    obj->push_signaler(this);
}

void Signaler::push_switch_mutual(Switch* obj) {
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

void Signaler::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    if (!(active_ && persistent_) && ((count_ >= threshold_) != active_)) {
        if (delta_frame) {
            delta_frame->push(std::make_unique<SignalerToggleDelta>(this));
        }
        active_ = !active_;
        for (Switchable* obj : switchables_) {
            obj->receive_signal(active_, room_map, delta_frame, mp);
        }
    }
}

void Signaler::serialize(MapFileO& file) {
    file << MapCode::Signaler;
    file << label_;
    file << count_ << threshold_ << persistent_ << active_;
    file << switches_.size();
    file << switchables_.size();
    for (auto& obj : switches_) {
        file << obj->pos();
    }
    for (auto& obj : switchables_) {
        file << obj->pos();
    }
}

void Signaler::remove_object(ObjectModifier* obj) {
    switchables_.erase(std::remove(switchables_.begin(), switchables_.end(), dynamic_cast<Switchable*>(obj)), switchables_.end());
    switches_.erase(std::remove(switches_.begin(), switches_.end(), dynamic_cast<Switch*>(obj)), switches_.end());
}
