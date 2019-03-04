#include "fallstepprocessor.h"

#include <algorithm>

#include "common.h"
#include "component.h"
#include "gameobject.h"
#include "roommap.h"
#include "delta.h"
#include "snakeblock.h"

// Move fall_check directly from MoveProcessor into FallStepProcessor
FallStepProcessor::FallStepProcessor(RoomMap* room_map, DeltaFrame* delta_frame, std::vector<GameObject*>&& fall_check):
fall_comps_unique_ {}, fall_check_ {fall_check}, snake_check_ {},
map_ {room_map}, delta_frame_ {delta_frame}, layers_fallen_ {} {}

FallStepProcessor::~FallStepProcessor() {}

// Returns whether anything falls
bool FallStepProcessor::run() {
    while (!fall_check_.empty()) {
        std::vector<GameObject*> next_fall_check {};
        for (GameObject* block : fall_check_) {
            if (!block->fall_comp() && block->tangible_) {
                auto comp_unique = std::make_unique<FallComponent>();
                FallComponent* comp = comp_unique.get();
                fall_comps_unique_.push_back(std::move(comp_unique));
                block->collect_sticky_component(map_, Sticky::All, comp);
                collect_above(comp, next_fall_check);
            }
        }
        fall_check_ = std::move(next_fall_check);
    }
    // Remove all components which have already landed
    for (auto& comp : fall_comps_unique_) {
        check_land_first(comp.get());
    }
    fall_comps_unique_.erase(std::remove_if(fall_comps_unique_.begin(), fall_comps_unique_.end(),
                                            [](auto& comp) { return comp->settled_; }), fall_comps_unique_.end());
    if (fall_comps_unique_.empty()) {
        return false;
    }
    // Collect all falling snakes, and their adjacent maybe-confused snakes
    for (auto& comp : fall_comps_unique_) {
        for (GameObject* block : comp->blocks_) {
            if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
                snake_check_.insert(sb);
                sb->collect_maybe_confused_neighbors(map_, snake_check_);
            }
        }
    }
    for (auto& comp : fall_comps_unique_) {
        comp->take_falling(map_);
    }
    layers_fallen_ = 0;
    while (true) {
        ++layers_fallen_;
        bool done_falling = true;
        for (auto& comp : fall_comps_unique_) {
            if (drop_check(comp.get())) {
                done_falling = false;
            }
        }
        if (done_falling) {
            break;
        }
        for (auto& comp : fall_comps_unique_) {
            if (!comp->settled_) {
                check_land_sticky(comp.get());
            }
        }
    }
    for (SnakeBlock* snake : snake_check_) {
        snake->check_add_local_links(map_, delta_frame_);
    }
    return true;
}

void FallStepProcessor::collect_above(FallComponent* comp, std::vector<GameObject*>& above_list) {
    for (GameObject* block : comp->blocks_) {
        GameObject* above = map_->view(block->shifted_pos({0,0,1}));
        if (above && above->gravitable_ && !above->fall_comp()) {
            above_list.push_back(above);
        }
    }
}

void FallStepProcessor::check_land_first(FallComponent* comp) {
    std::vector<FallComponent*> comps_below;
    for (GameObject* block : comp->blocks_) {
        if (!block->gravitable_) {
            comp->settle_first();
        }
        GameObject* below = map_->view(block->shifted_pos({0,0,-1}));
        if (below) {
            if (FallComponent* comp_below = below->fall_comp()) {
                if (!comp_below->settled_) {
                    if (comp_below != comp) {
                        comps_below.push_back(comp_below);
                    }
                    continue;
                }
            }
            comp->settle_first();
            return;
        }
    }
    for (FallComponent* comp_below : comps_below) {
        comp_below->add_above(comp);
    }
}

// Returns whether the component is "still falling" (false if stopped or reached oblivion)
bool FallStepProcessor::drop_check(FallComponent* comp) {
    if (comp->settled_) {
        return false;
    }
    bool alive = false;
    for (GameObject* block : comp->blocks_) {
        block->pos_ += {0,0,-1};
        if (block->pos_.z >= 0) {
            alive = true;
        }
    }
    if (!alive) {
        handle_fallen_blocks(comp);
    }
    return alive;
}

void FallStepProcessor::check_land_sticky(FallComponent* comp) {
    for (GameObject* block : comp->blocks_) {
        if (block->pos_.z < 0) {
            continue;
        }
        if (map_->view(block->shifted_pos({0,0,-1})) || block->has_sticky_neighbor(map_)) {
            settle(comp);
            return;
        }
    }
}

// TODO: Check snake links post-fall!!
void FallStepProcessor::handle_fallen_blocks(FallComponent* comp) {
    comp->settled_ = true;
    std::vector<GameObject*> live_blocks {};
    for (GameObject* block : comp->blocks_) {
        if (block->pos_.z >= 0) {
            // TODO: put the responsibility of making fall trails in a better place
            map_->make_fall_trail(block, layers_fallen_, 0);
            live_blocks.push_back(block);
            map_->put(block);
        } else {
            // NOTE: magic number for trail size
            map_->make_fall_trail(block, layers_fallen_, 10);
            block->pos_ += {0,0,layers_fallen_};
            map_->just_put(block);
            map_->destroy(block, delta_frame_);
            if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
                snake_check_.erase(sb);
            }
        }
    }
    if (!live_blocks.empty() && delta_frame_) {
        delta_frame_->push(std::make_unique<BatchMotionDelta>(std::move(live_blocks), Point3{0,0,-layers_fallen_}, map_));
    }
}

void FallStepProcessor::settle(FallComponent* comp) {
    handle_fallen_blocks(comp);
    for (FallComponent* above : comp->above_) {
        if (!above->settled_) {
            settle(comp);
        }
    }
}
