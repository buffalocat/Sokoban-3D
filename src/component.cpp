#include "component.h"
#include "roommap.h"
#include "block.h"
#include "delta.h"

Component::Component() {}

Component::~Component() {}


StrongComponent::StrongComponent(): Component(), weak_ {}, state_ {MoveComponentState::Contingent} {}

StrongComponent::~StrongComponent() {}

void StrongComponent::add_weak(StrongComponent* comp) {
    weak_.push_back(comp);
    comp->weak_.push_back(this);
}

bool StrongComponent::good() {
    return state_ == MoveComponentState::Good;
}

bool StrongComponent::bad() {
    return state_ == MoveComponentState::Bad;
}

void StrongComponent::set_bad() {
    state_ = MoveComponentState::Bad;
}

void StrongComponent::add_block(Block*) {}

ComplexComponent::ComplexComponent(): StrongComponent(), blocks_ {}, push_ {} {}

ComplexComponent::~ComplexComponent() {}

void ComplexComponent::resolve_contingent() {
    if (state_ == MoveComponentState::Contingent) {
        state_ = MoveComponentState::Good;
        for (auto& comp : push_) {
            comp->resolve_contingent();
        }
        for (auto& comp : weak_) {
            comp->resolve_contingent();
        }
    }
}

void ComplexComponent::collect_good(std::vector<Block*>& to_move) {
    if (state_ == MoveComponentState::Good) {
        for (Block* block : blocks_) {
            to_move.push_back(block);
        }
    }
}

void ComplexComponent::reset_blocks_comps() {
    for (Block* block : blocks_) {
        block->reset_comp();
    }
}

void ComplexComponent::add_block(Block* block) {
    blocks_.push_back(block);
}

void ComplexComponent::add_push(StrongComponent* comp) {
    push_.push_back(comp);
}

std::vector<Point3> ComplexComponent::to_push(Point3 d) {
    std::vector<Point3> pushes {};
    for (auto block : blocks_) {
        pushes.push_back(block->shifted_pos(d));
    }
    return pushes;
}

std::vector<Block*> ComplexComponent::get_weak_links(RoomMap* room_map) {
    std::vector<Block*> links {};
    for (auto block : blocks_) {
        block->get_weak_links(room_map, links);
    }
    return links;
}


SingletonComponent::SingletonComponent(Block* block):
StrongComponent(), block_ {block}, push_ {} {}

SingletonComponent::~SingletonComponent() {}

void SingletonComponent::add_push(StrongComponent* comp) {
    push_ = comp;
}

std::vector<Point3> SingletonComponent::to_push(Point3 d) {
    return {block_->shifted_pos(d)};
}

std::vector<Block*> SingletonComponent::get_weak_links(RoomMap* room_map) {
    std::vector<Block*> links {};
    block_->get_weak_links(room_map, links);
    return links;
}

void SingletonComponent::resolve_contingent() {
    if (state_ == MoveComponentState::Contingent) {
        state_ = MoveComponentState::Good;
        if (push_) {
            push_->resolve_contingent();
        }
        for (auto& comp : weak_) {
            comp->resolve_contingent();
        }
    }
}

void SingletonComponent::collect_good(std::vector<Block*>& to_move) {
    if (state_ == MoveComponentState::Good) {
        to_move.push_back(block_);
    }
}

void SingletonComponent::reset_blocks_comps() {
    block_->reset_comp();
}


WeakComponent::WeakComponent(): Component(),
blocks_ {}, unique_blocks_ {}, above_ {}, falling_ {true} {}

WeakComponent::~WeakComponent() {}

void WeakComponent::add_block(Block* block) {
    blocks_.push_back(block);
}

bool WeakComponent::falling() {
    return falling_;
}

void WeakComponent::collect_above(std::vector<Block*>& above_list, RoomMap* room_map) {
    for (Block* block : blocks_) {
        Block* above = dynamic_cast<Block*>(room_map->view(block->shifted_pos({0,0,1})));
        if (above && !above->w_comp()) {
            above_list.push_back(above);
        }
    }
}

void WeakComponent::collect_falling_unique(RoomMap* room_map) {
    if (!falling_) {
        return;
    }
    for (Block* block : blocks_) {
        unique_blocks_.push_back(std::move(room_map->take_quiet(block)));
    }
}

void WeakComponent::check_land_first(RoomMap* room_map) {
    std::vector<WeakComponent*> comps_below;
    for (Block* block : blocks_) {
        GameObject* obj = room_map->view(block->shifted_pos({0,0,-1}));
        if (obj) {
            Block* block_below = dynamic_cast<Block*>(obj);
            if (block_below) {
                WeakComponent* comp_below = block_below->w_comp();
                if (comp_below && comp_below->falling_ && comp_below != this) {
                    comps_below.push_back(comp_below);
                    continue;
                }
            }
            settle_first();
            return;
        }
    }
    for (WeakComponent* comp : comps_below) {
        comp->above_.push_back(this);
    }
}

void WeakComponent::settle_first() {
    falling_ = false;
    for (WeakComponent* comp : above_) {
        if (comp->falling_) {
            comp->settle_first();
        }
    }
}


void WeakComponent::reset_blocks_comps() {
    for (Block* block : blocks_) {
        block->reset_comp();
    }
}

// Returns whether the component is "still falling" (false if stopped or reached oblivion)
bool WeakComponent::drop_check(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame) {
    if (!falling_) {
        return false;
    }
    bool alive = false;
    for (Block* block : blocks_) {
        block->shift_pos({0,0,-1});
        if (block->z() >= 0) {
            alive = true;
        }
    }
    if (!alive) {
        handle_unique_blocks(layers_fallen, room_map, delta_frame);
    }
    return alive;
}

void WeakComponent::check_land_sticky(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame) {
    for (Block* block : blocks_) {
        if (block->z() < 0) {
            continue;
        }
        if (room_map->view(block->shifted_pos({0,0,-1})) || block->has_weak_neighbor(room_map)) {
            settle(layers_fallen, room_map, delta_frame);
            return;
        }
    }
}

void WeakComponent::handle_unique_blocks(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame) {
    falling_ = false;
    std::vector<Block*> live_blocks {};
    for (auto& block : unique_blocks_) {
        if (block->z() >= 0) {
            live_blocks.push_back(static_cast<Block*>(block.get()));
            auto obj = room_map->view(block->shifted_pos({0,0,-1}));
            room_map->put_quiet(std::move(block));
            if (obj) {
                obj->check_above_occupied(room_map, delta_frame);
            }
        } else {
            block->shift_pos({0,0,layers_fallen});
            if (delta_frame) {
                delta_frame->push(std::make_unique<DeletionDelta>(std::move(block), room_map));
            }
        }
    }
    if (!live_blocks.empty() && delta_frame) {
        delta_frame->push(std::make_unique<FallDelta>(std::move(live_blocks), layers_fallen, room_map));
    }
}

void WeakComponent::settle(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame) {
    handle_unique_blocks(layers_fallen, room_map, delta_frame);
    for (WeakComponent* comp : above_) {
        if (comp->falling_) {
            comp->settle(layers_fallen, room_map, delta_frame);
        }
    }
}
