#include "delta.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "snakeblock.h"
#include "player.h"
#include "switch.h"
#include "switchable.h"
#include "signaler.h"
#include "playingstate.h"

Delta::~Delta() {}


DeltaFrame::DeltaFrame(): deltas_ {} {}

DeltaFrame::~DeltaFrame() {}

void DeltaFrame::revert() {
    for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
        (**it).revert();
    }
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
    deltas_.push_back(std::move(delta));
}

bool DeltaFrame::trivial() {
    return deltas_.empty();
}


UndoStack::UndoStack(unsigned int max_depth): frames_ {}, max_depth_ {max_depth}, size_ {0} {}

UndoStack::~UndoStack() {}

void UndoStack::push(std::unique_ptr<DeltaFrame> delta_frame) {
    if (!delta_frame->trivial()) {
        if (size_ == max_depth_) {
            frames_.pop_front();
        } else {
            ++size_;
        }
        frames_.push_back(std::move(delta_frame));
    }
}

bool UndoStack::non_empty() {
    return size_ > 0;
}

void UndoStack::pop() {
    frames_.back()->revert();
    frames_.pop_back();
    --size_;
}

void UndoStack::reset() {
    frames_.clear();
    size_ = 0;
}


CreationDelta::CreationDelta(GameObject* obj, RoomMap* room_map): obj_ {obj}, map_ {room_map} {}

CreationDelta::~CreationDelta() {}

void CreationDelta::revert() {
    map_->uncreate(obj_);
}


DeletionDelta::DeletionDelta(GameObject* obj, RoomMap* room_map):
obj_ {obj}, map_ {room_map} {}

DeletionDelta::~DeletionDelta() {}

void DeletionDelta::revert() {
    map_->undestroy(obj_);
}


PutDelta::PutDelta(GameObject* obj, RoomMap* room_map):
obj_ {obj}, map_ {room_map} {}

PutDelta::~PutDelta() {}

void PutDelta::revert() {
    map_->just_take(obj_);
}


// NOTE: A Taken (intangible) object won't have its position updated
// until it has been Put back into the map, so there's no need
// to record its old position in the delta.
TakeDelta::TakeDelta(GameObject* obj, RoomMap* room_map):
obj_ {obj}, map_ {room_map} {}

TakeDelta::~TakeDelta() {}

void TakeDelta::revert() {
    map_->just_put(obj_);
}


MotionDelta::MotionDelta(GameObject* obj, Point3 dpos, RoomMap* room_map):
obj_ {obj}, dpos_ {dpos}, map_ {room_map} {}

MotionDelta::~MotionDelta() {}

void MotionDelta::revert() {
    map_->just_shift(obj_, -dpos_);
}


BatchMotionDelta::BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* room_map):
objs_ {objs}, dpos_ {dpos}, map_ {room_map} {}

BatchMotionDelta::~BatchMotionDelta() {}

void BatchMotionDelta::revert() {
    map_->just_batch_shift(objs_, -dpos_);
}


AbstractMotionDelta::AbstractMotionDelta(GameObject* obj, Point3 dpos):
obj_ {obj}, dpos_ {dpos} {}

AbstractMotionDelta::~AbstractMotionDelta() {}

void AbstractMotionDelta::revert() {
    obj_->pos_ -= dpos_;
}


AddLinkDelta::AddLinkDelta(SnakeBlock* a, SnakeBlock* b): a_ {a}, b_ {b} {}

AddLinkDelta::~AddLinkDelta() {}

void AddLinkDelta::revert() {
    a_->remove_link_quiet(b_);
}


RemoveLinkDelta::RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b): a_ {a}, b_ {b} {}

RemoveLinkDelta::~RemoveLinkDelta() {}

void RemoveLinkDelta::revert() {
    a_->add_link_quiet(b_);
}


DoorMoveDelta::DoorMoveDelta(PlayingState* state, Room* room, Point3 pos):
state_ {state}, room_ {room}, pos_ {pos} {}

DoorMoveDelta::~DoorMoveDelta() {}

void DoorMoveDelta::revert() {
    RoomMap* cur_map = state_->room_->room_map();
    RoomMap* dest_map = room_->room_map();
    Player* player = state_->player_;
    state_->room_ = room_;
    cur_map->take(player);
    if (Car* car = player->get_car(cur_map, true)) {
        GameObject* car_concrete = car->parent_;
        cur_map->take(car_concrete);
        car_concrete->pos_ = pos_ + Point3{0,0,-1};
        dest_map->put(car_concrete);
    }
    player->pos_ = pos_;
    dest_map->put(player);
}


SwitchableDelta::SwitchableDelta(Switchable* obj, bool active, bool waiting):
obj_ {obj}, active_ {active}, waiting_ {waiting} {}

SwitchableDelta::~SwitchableDelta() {}

void SwitchableDelta::revert() {
    obj_->active_ = active_;
    obj_->waiting_ = waiting_;
}


SwitchToggleDelta::SwitchToggleDelta(Switch* obj): obj_ {obj} {}

SwitchToggleDelta::~SwitchToggleDelta() {}

void SwitchToggleDelta::revert() {
    obj_->toggle();
}


SignalerToggleDelta::SignalerToggleDelta(Signaler* sig): sig_ {sig} {}

SignalerToggleDelta::~SignalerToggleDelta() {}

void SignalerToggleDelta::revert() {
    sig_->toggle();
}


RidingStateDelta::RidingStateDelta(Player* player, RidingState state):
player_ {player}, state_ {state} {}

RidingStateDelta::~RidingStateDelta() {}

void RidingStateDelta::revert() {
    player_->state_ = state_;
}


ColorChangeDelta::ColorChangeDelta(Car* car): car_ {car} {}

ColorChangeDelta::~ColorChangeDelta() {}

void ColorChangeDelta::revert() {
    car_->cycle_color(true);
}
