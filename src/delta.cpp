#include "delta.h"
#include "gameobject.h"
#include "roommap.h"
#include "block.h"
#include "room.h"

#include <iostream>

UndoStack::UndoStack(unsigned int max_depth): max_depth_ {max_depth}, size_ {0}, frames_ {} {}

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

DeltaFrame::DeltaFrame(): deltas_ {} {}

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

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object, RoomMap* room_map): object_ {std::move(object)}, room_map_ {room_map} {}

void DeletionDelta::revert() {
    GameObject* obj = object_.get();
    room_map_->put_quiet(std::move(object_));
    obj->reinit();
}

CreationDelta::CreationDelta(GameObject* object, RoomMap* room_map): object_ {object}, room_map_ {room_map} {}

void CreationDelta::revert() {
    room_map_->take_quiet(object_);
}

MotionDelta::MotionDelta(Block* object, Point p, RoomMap* room_map): object_ {object}, p_ {p}, room_map_ {room_map} {}

void MotionDelta::revert() {
    auto object_unique = room_map_->take_quiet(object_);
    object_->set_pos(p_);
    room_map_->put_quiet(std::move(object_unique));
}

AddLinkDelta::AddLinkDelta(Block* a, Block* b): a_ {a}, b_ {b} {}

void AddLinkDelta::revert() {
    a_->remove_link(b_, nullptr);
}

RemoveLinkDelta::RemoveLinkDelta(Block* a, Block* b): a_ {a}, b_ {b} {}

void RemoveLinkDelta::revert() {
    a_->add_link(b_, nullptr);
}

DoorMoveDelta::DoorMoveDelta(RoomManager* mgr, Room* prev_room, Point pos, Block* player):
    mgr_ {mgr}, prev_room_ {prev_room}, pos_ {pos}, player_ {player} {}

void DoorMoveDelta::revert() {
    auto player_unique = mgr_->room_map()->take_quiet(player_);
    mgr_->set_cur_room(prev_room_);
    player_->set_pos(pos_);
    mgr_->room_map()->put_quiet(std::move(player_unique));
}
