#include "delta.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "block.h"
#include "snakeblock.h"
#include "player.h"
#include "switch.h"
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


UndoStack::UndoStack(unsigned int max_depth): max_depth_ {max_depth}, size_ {0}, frames_ {} {}

UndoStack::~UndoStack() {}

void UndoStack::push(std::unique_ptr<DeltaFrame> delta_frame) {
    if (!(*delta_frame).trivial()) {
        if (size_ == max_depth_) {
            frames_.pop_front();
        } else {
            ++size_;
        }
        frames_.push_back(std::move(delta_frame));
    }
}

bool UndoStack::pop() {
    if (size_ > 0) {
        (*frames_.back()).revert();
        frames_.pop_back();
        --size_;
        return true;
    }
    return false;
}

void UndoStack::reset() {
    frames_.clear();
    size_ = 0;
}

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object, RoomMap* room_map): object_ {std::move(object)}, room_map_ {room_map} {}

DeletionDelta::~DeletionDelta() {}

void DeletionDelta::revert() {
    GameObject* obj = object_.get();
    room_map_->put_quiet(std::move(object_));
}


CreationDelta::CreationDelta(Point3 pos, RoomMap* room_map): pos_ {pos}, room_map_ {room_map} {}

CreationDelta::~CreationDelta() {}

void CreationDelta::revert() {
    room_map_->take_quiet(pos_);
}


MotionDelta::MotionDelta(GameObject* object, Point3 p, RoomMap* room_map): object_ {object}, p_ {p}, room_map_ {room_map} {}

MotionDelta::~MotionDelta() {}

void MotionDelta::revert() {
    object_->set_pos_auto(p_, room_map_, nullptr);
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
    Block* car = player->get_car(cur_map, true);
    state_->room_ = room_;
    auto player_unique = cur_map->take_quiet(player);
    if (car) {
        auto car_unique = cur_map->take_quiet(car);
        car->set_pos(pos_);
        dest_map->put_quiet(std::move(car_unique));
    }
    player->set_pos(pos_);
    dest_map->put_quiet(std::move(player_unique));
}


SwitchableDelta::SwitchableDelta(Switchable* obj, bool active, bool waiting):
obj_ {obj}, active_ {active}, waiting_ {waiting} {}

SwitchableDelta::~SwitchableDelta() {}

void SwitchableDelta::revert() {
    obj_->set_aw(active_, waiting_);
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

ColorChangeDelta::ColorChangeDelta(Block* obj): obj_ {obj} {}

ColorChangeDelta::~ColorChangeDelta() {}

void ColorChangeDelta::revert() {
    obj_->cycle_color(true);
}
