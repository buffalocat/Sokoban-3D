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

//TODO: make sure DeletionDelta is right
DeletionDelta::DeletionDelta(GameObject* obj, RoomMap* room_map):
obj_ {obj}, room_map_ {room_map} {
    obj_->cleanup_on_destruction(room_map);
}

DeletionDelta::~DeletionDelta() {}

void DeletionDelta::revert() {
    obj_->setup_on_undestruction(room_map_);
    room_map_->put(obj_);
}


CreationDelta::CreationDelta(GameObject* obj, RoomMap* room_map): obj_ {obj}, room_map_ {room_map} {}

CreationDelta::~CreationDelta() {}

void CreationDelta::revert() {
    room_map_->take(obj_);
}


MotionDelta::MotionDelta(GameObject* obj, Point3 dpos, RoomMap* room_map):
obj_ {obj}, dpos_ {dpos}, room_map_ {room_map} {}

MotionDelta::~MotionDelta() {}

void MotionDelta::revert() {
    room_map_->shift(obj_, -dpos_, nullptr);
}


BatchMotionDelta::BatchMotionDelta(std::vector<GameObject*> objs, Point3 dpos, RoomMap* room_map):
objs_ {objs}, dpos_ {dpos}, room_map_ {room_map} {}

BatchMotionDelta::~BatchMotionDelta() {}

void BatchMotionDelta::revert() {
    room_map_->batch_shift(objs_, -dpos_, nullptr);
}

AddLinkDelta::AddLinkDelta(SnakeBlock* a, SnakeBlock* b): a_ {a}, b_ {b} {}

AddLinkDelta::~AddLinkDelta() {}

void AddLinkDelta::revert() {
    a_->remove_link(b_, nullptr);
}

RemoveLinkDelta::RemoveLinkDelta(SnakeBlock* a, SnakeBlock* b): a_ {a}, b_ {b} {}

RemoveLinkDelta::~RemoveLinkDelta() {}

void RemoveLinkDelta::revert() {
    a_->add_link(b_, nullptr);
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


SwitchableDelta::SwitchableDelta(Switchable* obj, bool active, bool waiting, RoomMap* room_map):
obj_ {obj}, room_map_ {room_map}, active_ {active}, waiting_ {waiting} {}

SwitchableDelta::~SwitchableDelta() {}

void SwitchableDelta::revert() {
    obj_->set_aw(active_, waiting_, room_map_);
}


SwitchToggleDelta::SwitchToggleDelta(Switch* obj): obj_ {obj} {}

SwitchToggleDelta::~SwitchToggleDelta() {}

void SwitchToggleDelta::revert() {
    obj_->toggle();
}


SignalerToggleDelta::SignalerToggleDelta(Signaler* obj): obj_ {obj} {}

SignalerToggleDelta::~SignalerToggleDelta() {}

void SignalerToggleDelta::revert() {
    obj_->toggle();
}


RidingStateDelta::RidingStateDelta(Player* player, RidingState state):
player_ {player}, state_ {state} {}

RidingStateDelta::~RidingStateDelta() {}

void RidingStateDelta::revert() {
    player_->set_riding(state_);
}


ColorChangeDelta::ColorChangeDelta(Car* car): car_ {car} {}

ColorChangeDelta::~ColorChangeDelta() {}

void ColorChangeDelta::revert() {
    car_->cycle_color(true);
}
