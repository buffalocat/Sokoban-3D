#include "delta.h"
#include "gameobject.h"
#include "roommap.h"
#include "block.h"

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

bool UndoStack::pop(RoomMap* room_map) {
    if (size_ > 0) {
        (*frames_.back()).revert(room_map);
        frames_.pop_back();
        --size_;
        return true;
    }
    return false;
}

DeltaFrame::DeltaFrame(): deltas_ {} {}

void DeltaFrame::revert(RoomMap* room_map) {
    for (auto it = deltas_.rbegin(); it != deltas_.rend(); ++it) {
        (**it).revert(room_map);
    }
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
    deltas_.push_back(std::move(delta));
}

bool DeltaFrame::trivial() {
    return deltas_.empty();
}

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object): object_ {std::move(object)} {}

void DeletionDelta::revert(RoomMap* room_map) {
    GameObject* obj = object_.get();
    room_map->put_quiet(std::move(object_));
    obj->reinit();
}

CreationDelta::CreationDelta(GameObject* object): object_ {object} {}

void CreationDelta::revert(RoomMap* room_map) {
    room_map->take_quiet(object_);
}

MotionDelta::MotionDelta(Block* object, Point p): object_ {object}, p_ {p} {}

void MotionDelta::revert(RoomMap* room_map) {
    auto object_unique = room_map->take_quiet(object_);
    object_->set_pos(p_, nullptr);
    room_map->put_quiet(std::move(object_unique));
}

AddLinkDelta::AddLinkDelta(Block* a, Block* b): a_ {a}, b_ {b} {}

void AddLinkDelta::revert(RoomMap* room_map) {
    a_->remove_link(b_, nullptr);
}

RemoveLinkDelta::RemoveLinkDelta(Block* a, Block* b): a_ {a}, b_ {b} {}

void RemoveLinkDelta::revert(RoomMap* room_map) {
    a_->add_link(b_, nullptr);
}
