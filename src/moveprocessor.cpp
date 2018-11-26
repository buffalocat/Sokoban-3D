#include "moveprocessor.h"
#include "block.h"
#include "player.h"
#include "delta.h"
#include "roommap.h"
#include "switch.h"
#include "component.h"

MoveProcessor::MoveProcessor(Player* player, RoomMap* room_map, Point3 dir):
player_ {player}, map_ {room_map}, dir_ {dir}, comps_ {} {}

MoveProcessor::~MoveProcessor() {}

void MoveProcessor::try_move(DeltaFrame* delta_frame) {
    if (player_->state() == RidingState::Bound) {
        move_bound(delta_frame);
    } else {
        move_general(delta_frame);
    }
}

void MoveProcessor::move_bound(DeltaFrame* delta_frame) {
    // This is more complicated in 3D...
    // For now, don't let bound player push anything
    if (map_->view(player_->shifted_pos(dir_))) {
        return;
    }
    // If the player is bound, it's on top of a block!
    Block* car = static_cast<Block*>(map_->view(player_->shifted_pos({0,0,-1})));
    Block* adj = dynamic_cast<Block*>(map_->view(car->shifted_pos(dir_)));
    if (adj && car->color() == adj->color()) {
        player_->shift_pos_auto(dir_, map_, delta_frame);
    }
}

void MoveProcessor::move_general(DeltaFrame* delta_frame) {
    std::vector<Component*> roots {};
    auto player_comp = player_->make_strong_component(map_);
    roots.push_back(player_comp.get());
    comps_.push_back(std::move(player_comp));
    Block* car = player_->get_car(map_, true);
    if (car) {
        auto car_comp = car->make_strong_component(map_);
        roots.push_back(car_comp.get());
        comps_.push_back(std::move(car_comp));
    }
    // When relevant, create other root components
    for (Component* comp : roots) {
        move_component(comp);
    }
    for (Component* comp : roots) {
        comp->resolve_contingent();
    }
    if (car && !(car->comp()->good() && player_->comp()->good())) {
        // Depending on what objects are in play,
        // some things may still move... but not for now
        return;
    }
    std::vector<Block*> to_move {};
    for (auto& comp : comps_) {
        comp->clean_up(to_move);
    }

}

bool MoveProcessor::move_component(Component* comp) {
    for (Point3 pos : comp->to_push(dir_)) {
        if (!try_push(comp, pos)) {
            comp->set_bad();
            return false;
        }
    }
    for(Block* link : comp->get_weak_links(map_)) {
        if (!link->comp()) {
            auto link_comp = link->make_strong_component(map_);
            move_component(link_comp.get());
            comps_.push_back(std::move(link_comp));
        }
        if (!link->comp()->bad()) {
            comp->add_weak(link->comp());
        }
    }
    return true;
}

bool MoveProcessor::try_push(Component* comp, Point3 pos) {
    if (!map_->valid(pos.h())) {
        return false;
    }
    GameObject* obj = map_->view(pos);
    if (!obj) {
        return true;
    }
    Block* block = dynamic_cast<Block*>(obj);
    if (!block) {
        return false;
    }
    if (block->comp()) {
        return !block->comp()->bad();
    }
    auto unique_comp = block->make_strong_component(map_);
    Component* pushed_comp = unique_comp.get();
    comps_.push_back(std::move(unique_comp));
    comp->add_push(pushed_comp);
    return move_component(pushed_comp);
}
