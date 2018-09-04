#include "delta.h"
#include "gameobject.h"
#include "worldmap.h"

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

void UndoStack::pop(WorldMap* world_map) {
    if (size_ > 0) {
        (*frames_.back()).revert(world_map);
        frames_.pop_back();
        --size_;
    }
}

DeltaFrame::DeltaFrame(): deltas_ {} {}

void DeltaFrame::revert(WorldMap* world_map) {
    for (auto& delta : deltas_) {
        (*delta).revert(world_map);
    }
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
    deltas_.push_back(std::move(delta));
}

bool DeltaFrame::trivial() {
    return deltas_.empty();
}

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object): object_ {std::move(object)} {}

void DeletionDelta::revert(WorldMap* world_map) {
    world_map->put_quiet(std::move(object_));
}

CreationDelta::CreationDelta(GameObject const& object): pos_ {object.pos()}, layer_ {object.layer()}, id_ {object.id()} {}

void CreationDelta::revert(WorldMap* world_map) {
    world_map->take_quiet(pos_, layer_, id_);
}

MotionDelta::MotionDelta(Block* object, Point d): object_ {object}, d_ {d} {}

void MotionDelta::revert(WorldMap* world_map) {
    auto object_unique = world_map->take_quiet(object_->pos(), object_->layer(), object_->id());
    object_->shift_pos(Point{-d_.x, -d_.y}, nullptr);
    world_map->put_quiet(std::move(object_unique));
}