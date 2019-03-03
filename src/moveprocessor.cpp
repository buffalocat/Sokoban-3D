#include "moveprocessor.h"

#include <iostream>

#include "common.h"
#include "gameobject.h"
#include "player.h"
#include "delta.h"
#include "roommap.h"
#include "switch.h"

#include "horizontalstepprocessor.h"
#include "fallstepprocessor.h"

MoveProcessor::MoveProcessor(RoomMap* room_map, DeltaFrame* delta_frame):
fall_check_ {}, moving_blocks_ {},
map_ {room_map}, delta_frame_ {delta_frame},
frames_ {0}, state_ {} {}

MoveProcessor::~MoveProcessor() {}

bool MoveProcessor::try_move(Player* player, Point3 dir) {
    if (!delta_frame_) {
        throw NullDeltaFrameException {};
    }
    state_ = MoveStep::Horizontal;
    if (player->state_ == RidingState::Bound) {
        move_bound(player, dir);
    } else {
        move_general(dir);
    }
    if (moving_blocks_.empty()) {
        return false;
    }
    frames_ = MOVEMENT_FRAMES;
    return true;
}

void MoveProcessor::move_bound(Player* player, Point3 dir) {
    // When Player is Bound, no other agents move
    if (map_->view(player->shifted_pos(dir))) {
        return;
    }
    // If the player is bound, it's on top of a block!
    GameObject* car = map_->view(player->shifted_pos({0,0,-1}));
    GameObject* adj = map_->view(car->shifted_pos(dir));
    if (adj && car->color_ == adj->color_) {
        map_->take(player);
        player->set_linear_animation(dir);
        delta_frame_->push(std::make_unique<MotionDelta>(player, dir, map_));
        moving_blocks_.push_back(player);
        player->shift_pos_from_animation();
        map_->put(player);
    }
}

void MoveProcessor::move_general(Point3 dir) {
    map_->print_snakes();
    HorizontalStepProcessor(map_, delta_frame_, dir, fall_check_, moving_blocks_).run();
}

bool MoveProcessor::update() {
    if (--frames_ == 0) {
        perform_switch_checks();
        begin_fall_cycle();
        return true;
    }
    for (GameObject* block : moving_blocks_) {
        block->update_animation();
    }
    return false;
}

void MoveProcessor::abort() {
    for (GameObject* block : moving_blocks_) {
        block->reset_animation();
    }
}

void MoveProcessor::color_change(Player* player) {
    if (!delta_frame_) {
        throw NullDeltaFrameException {};
    }
    Car* car = player->get_car(map_, false);
    if (!(car && car->cycle_color(false))) {
        return;
    }
    delta_frame_->push(std::make_unique<ColorChangeDelta>(car));
    fall_check_.push_back(car->parent_);
    for (Point3 d : DIRECTIONS) {
        if (GameObject* block = map_->view(car->shifted_pos(d))) {
            fall_check_.push_back(block);
        }
    }
    begin_fall_cycle();
}

void MoveProcessor::begin_fall_cycle() {
    if (!delta_frame_) {
        throw NullDeltaFrameException {};
    }
    state_ = MoveStep::Fall;
    // TODO: "split" this loop to allow for animation in between fall steps!
    while (!fall_check_.empty()) {
        FallStepProcessor(map_, delta_frame_, std::move(fall_check_)).run();
        fall_check_.clear();
        perform_switch_checks();
    }
}

void MoveProcessor::perform_switch_checks() {
    if (!delta_frame_) {
        throw NullDeltaFrameException {};
    }
    map_->alert_activated_listeners(delta_frame_, this);
    map_->reset_local_state();
    map_->check_signalers(delta_frame_, this);
}

void MoveProcessor::add_to_fall_check(GameObject* obj) {
    fall_check_.push_back(obj);
}
