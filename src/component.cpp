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

void StrongComponent::set_pushed() {}

bool StrongComponent::push_recheck() {
    return false;
}


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


SnakeComponent::SnakeComponent(Block* block): SingletonComponent(block), pushed_ {false} {}

SnakeComponent::~SnakeComponent() {}

void SnakeComponent::set_pushed() {
    pushed_ = true;
}

bool SnakeComponent::push_recheck() {
    if (pushed_) {
        return false;
    } else {
        pushed_ = true;
        return true;
    }
}

std::vector<Block*> SnakeComponent::get_weak_links(RoomMap* room_map) {
    std::vector<Block*> links {};
    // A snake block that wasn't pushed won't pull its links forward
    if (pushed_) {
        return block_->get_weak_links(room_map, links);
    }
    return links;
}

void SnakeComponent::pull_snakes(RoomMap* room_map, DeltaFrame* delta_frame, std::vector<std::pair<SnakeBlock*, Point3>>& pull_snakes, std::vector<SnakeBlock*>& check_snakes) {
    SnakeBlock* prev = static_cast<SnakeBlock*>(block_);
    check_snakes.push_back(prev);
    if (pushed_) {
        return;
    }
    SnakeBlock* cur {};
    for (auto link : prev->links) {
        if (!link->s_comp()) {
            cur = link;
        }
    }
    while (cur) {
        // If we reach the end of the snake, we can pull it
        if (cur->links_.size() == 1) {
            check_.insert(cur);
            pull(cur);
            break;
        }
        // Progress down the snake
        for (auto link : cur->links_) {
            if (link != prev) {
                cur->distance_ = d++;
                prev = cur;
                cur = static_cast<SnakeBlock*>(link);
                break;
            }
        }
        // If we reach a block with an initialized but shorter distance, we're done
        if (cur->distance_ >= 0 && d >= cur->distance_) {
            // The chain was so short that it didn't break (it was all pushed)!
            if (cur->distance_ <= 1) {
                break;
            }
            // The chain was odd length; split the middle block!
            else if (d == cur->distance_) {
                Point pos = cur->pos();
                room_map_->take(cur, delta_frame_);

                auto a_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, cur->color_, false, 1);
                auto a = a_unique.get();
                room_map_->put(std::move(a_unique), delta_frame_);
                a->target_ = prev;
                a->add_link(prev, delta_frame_);
                pull(a);

                auto b_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, cur->color_, false, 1);
                auto b = b_unique.get();
                room_map_->put(std::move(b_unique), delta_frame_);
                b->target_ = cur->target_;
                b->add_link(cur->target_, delta_frame_);
                pull(b);

                // This snake won't get its target reset otherwise
                // This causes problems post "resurrection" from undo!
                cur->reset_target();
            }
            // The chain was even length; cut!
            else {
                cur->remove_link(prev, delta_frame_);
                pull(cur);
                pull(prev);
            }
            break;
        }
        cur->target_ = prev;
    }
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
