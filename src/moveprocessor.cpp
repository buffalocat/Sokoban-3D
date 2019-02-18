#include "moveprocessor.h"

#include "common.h"

#include "gameobject.h"
#include "snakeblock.h"
#include "player.h"
#include "delta.h"
#include "roommap.h"
#include "switch.h"
#include "component.h"

MoveProcessor::MoveProcessor(Player* player, RoomMap* room_map, Point3 dir, DeltaFrame* delta_frame):
push_comps_unique_ {}, fall_comps_unique_ {},
moving_blocks_ {}, fall_check_ {}, link_break_check_ {},
moving_snakes_ {}, snakes_to_reset_ {}, snakes_to_recheck_ {},
player_ {player}, map_ {room_map}, delta_frame_ {delta_frame}, dir_ {dir},
frames_ {0}, state_ {MoveStepType::Horizontal} {}

MoveProcessor::~MoveProcessor() {}

bool MoveProcessor::try_move() {
    if (player_->state() == RidingState::Bound) {
        move_bound();
    } else {
        move_general();
    }
    if (moving_blocks_.empty()) {
        return false;
    }
    frames_ = MOVEMENT_FRAMES;
    return true;
}

void MoveProcessor::move_bound() {
    // This is more complicated in 3D...
    // For now, don't let bound player push anything
    if (map_->view(player_->shifted_pos(dir_))) {
        return;
    }
    // If the player is bound, it's on top of a block!
    GameObject* car = map_->view(player_->shifted_pos({0,0,-1}));
    GameObject* adj = map_->view(car->shifted_pos(dir_));
    if (adj && car->color_ == adj->color_) {
        map_->take(player_);
        player_->set_linear_animation(dir_);
        if (delta_frame_) {
            delta_frame_->push(std::make_unique<MotionDelta>(player_, dir_, map_));
        }
        moving_blocks_.push_back(player_);
        player_->shift_pos_from_animation();
        map_->put(player_);
    }
}

void MoveProcessor::move_general() {
    prepare_horizontal_move();
    perform_horizontal_step();
    push_comps_unique_.clear();
}


bool MoveProcessor::update() {
    if (--frames_ == 0) {
        perform_switch_checks();
        begin_fall_cycle();
        return true;
    } else {
        for (GameObject* block : moving_blocks_) {
            block->update_animation();
        }
    }
    return false;
}

void MoveProcessor::abort() {
    for (GameObject* block : moving_blocks_) {
        block->reset_animation();
    }
}

void MoveProcessor::color_change_check() {
    Car* car = player_->get_car(map_, false);
    if (!(car && car->cycle_color(false))) {
        return;
    }
    if (delta_frame_) {
        delta_frame_->push(std::make_unique<ColorChangeDelta>(car));
    }
    fall_check_.push_back(car->parent_);
    for (Point3 d : DIRECTIONS) {
        if (GameObject* block = map_->view(car->parent_->shifted_pos(d))) {
            fall_check_.push_back(block);
        }
    }
    begin_fall_cycle();
}

// Try to push the block and build the resulting component tree
// Return whether block is able to move
bool MoveProcessor::compute_push_component_tree(GameObject* block) {
    snakes_to_reset_ = {};
    snakes_to_recheck_ = {};
    if (!compute_push_component(block)) {
        for (auto snake : snakes_to_reset_) {
            // "Unpush" snakes that we thought we pushed, but didn't really
            snake->toggle_push();
        }
        return false;
    }
    std::vector<GameObject*> weak_links {};
    // Ensures that snakes which were "pushed late" still drag their links
    for (auto snake : snakes_to_recheck_) {
        snake->collect_sticky_links(map_, Sticky::Snake, weak_links);
    }
    collect_moving_and_weak_links(block->push_comp(), weak_links);
    for (auto link : weak_links) {
        if (!compute_push_component_tree(link)) {
            if (auto sb = dynamic_cast<SnakeBlock*>(link)) {
                link_break_check_.push_back(sb);
            }
        }
    }
    return true;
}

// Try to push the component containing block
// Return whether block is able to move
bool MoveProcessor::compute_push_component(GameObject* start_block) {
    if (PushComponent* comp = start_block->push_comp()) {
        return !comp->blocked_;
    }
    auto comp_unique = std::make_unique<PushComponent>();
    PushComponent* comp = comp_unique.get();
    push_comps_unique_.push_back(std::move(comp_unique));
    start_block->collect_sticky_component(map_, Sticky::Strong, comp);
    for (auto block : comp->blocks_) {
        if (!block->pushable_) {
            comp->blocked_ = true;
            break;
        }
        if (GameObject* in_front = map_->view(block->pos_ + dir_)) {
            if (in_front->pushable_ && compute_push_component(in_front)) {
                comp->add_pushing(in_front->push_comp());
                if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(in_front)) {
                    sb->toggle_push();
                    snakes_to_reset_.push_back(sb);
                    if (sb->pushed_and_moving()) {
                        snakes_to_recheck_.push_back(sb);
                    }
                }
            } else {
                // The thing we tried to push wasn't pushable or couldn't move
                comp->blocked_ = true;
                break;
            }
        }
    }
    return !comp->blocked_;
}


void MoveProcessor::collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links) {
    if (comp->moving_) {
        return;
    }
    comp->moving_ = true;
    for (GameObject* block : comp->blocks_) {
        moving_blocks_.push_back(block);
        if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
            sb->record_move();
            // Only pushed snakes should drag their links
            Sticky sticky_condition = sb->pushed_and_moving() ? Sticky::SnakeWeak : Sticky::Weak;
            block->collect_sticky_links(map_, sticky_condition, weak_links);
            moving_snakes_.push_back(sb);
        } else {
            block->collect_sticky_links(map_, Sticky::Weak, weak_links);
        }
    }
    for (PushComponent* in_front : comp->pushing_) {
        collect_moving_and_weak_links(in_front, weak_links);
    }
}

void MoveProcessor::prepare_horizontal_move() {
    // TODO: allow for possibility of multiple/dependent agents
    compute_push_component_tree(player_);
}

void MoveProcessor::perform_horizontal_step() {
    // Any block which moved forward could have moved off a ledge
    fall_check_ = moving_blocks_;
    // Keep a list of snakes which did not move but may have gained links anyway
    std::unordered_set<SnakeBlock*> link_add_check {};
    for (auto sb : link_break_check_) {
        fall_check_.push_back(sb);
        // NOTE: May be made redundant by the link_add_check insertion for all moving_snakes_!
        link_add_check.insert(sb);
        sb->remove_moving_links(delta_frame_);
    }
    SnakePuller snake_puller {map_, delta_frame_, moving_snakes_, link_add_check, fall_check_};
    for (auto sb : moving_snakes_) {
        snake_puller.prepare_pull(sb);
    }
    // TODO: Move this inside prepare_pull() (don't forget to include tails!)
    // BIGGER TODO: Figure out the failure mode of Weird Stuff happening when
    // stuff gets pulled in a weird direction
    /*
    for (auto sb : moving_snakes_) {
        sb->collect_maybe_confused_links(map_, link_add_check);
    }
    */
    for (auto block : moving_blocks_) {
        block->set_linear_animation(dir_);
    }
    // MAP BECOMES INCONSISTENT HERE (potential ID overlap)
    snake_puller.perform_pulls();
    map_->batch_shift(std::move(moving_blocks_), dir_, delta_frame_);
    // MAP BECOMES CONSISTENT AGAIN HERE
    for (auto sb : link_add_check) {
        sb->check_add_local_links(map_, delta_frame_);
    }
}

void MoveProcessor::perform_switch_checks() {
    map_->alert_activated_listeners(delta_frame_, this);
    map_->check_signalers(delta_frame_, this);
}

void MoveProcessor::begin_fall_cycle() {
    // TODO: "split" this loop to allow for animation in between fall steps!
    while (!fall_check_.empty()) {
        fall_step();
        fall_comps_unique_.clear();
        fall_check_.clear();
        map_->check_signalers(delta_frame_, this);
    }
}

void MoveProcessor::fall_step() {
    while (!fall_check_.empty()) {
        std::vector<GameObject*> next_fall_check {};
        for (GameObject* block : fall_check_) {
            if (!block->fall_comp()) {
                auto comp_unique = std::make_unique<FallComponent>();
                FallComponent* comp = comp_unique.get();
                fall_comps_unique_.push_back(std::move(comp_unique));
                block->collect_sticky_component(map_, Sticky::All, comp);
                collect_above(comp, next_fall_check);
            }
        }
        fall_check_ = std::move(next_fall_check);
    }
    // Initial check for land
    for (auto& comp : fall_comps_unique_) {
        check_land_first(comp.get());
    }
    for (auto& comp : fall_comps_unique_) {
        comp->collect_falling_unique(map_);
    }
    int layers_fallen = 0;
    while (true) {
        ++layers_fallen;
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
}

void MoveProcessor::collect_above(FallComponent* comp, std::vector<GameObject*>& above_list) {
    for (GameObject* block : comp->blocks_) {
        GameObject* above = map_->view(block->shifted_pos({0,0,1}));
        if (above && above->gravitable_ && !above->fall_comp()) {
            above_list.push_back(above);
        }
    }
}

void MoveProcessor::check_land_first(FallComponent* comp) {
    std::vector<FallComponent*> comps_below;
    for (GameObject* block : comp->blocks_) {
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

Component::~Component() {
    for (GameObject* block : blocks_) {
        block->comp_ = nullptr;
    }
}

void FallComponent::settle_first() {
    settled_ = true;
    for (FallComponent* comp : above_) {
        if (!comp->settled_) {
            comp->settle_first();
        }
    }
}

void FallComponent::collect_falling_unique(RoomMap* room_map) {
    if (settled_) {
        return;
    }
    for (GameObject* block : blocks_) {
        room_map->take(block);
    }
}

// Returns whether the component is "still falling" (false if stopped or reached oblivion)
bool MoveProcessor::drop_check(FallComponent* comp) {
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
        handle_unique_blocks(comp);
    }
    return alive;
}

void MoveProcessor::check_land_sticky(FallComponent* comp) {
    for (GameObject* block : comp->blocks_) {
        // NOTE: make this more flexible if gravity ever changes!
        if (block->pos_.z < 0) {
            continue;
        }
        if (map_->view(block->shifted_pos({0,0,-1})) || block->has_sticky_neighbor(map_)) {
            settle(comp);
            return;
        }
    }
}

void MoveProcessor::handle_unique_blocks(FallComponent* comp) {
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
            if (delta_frame_) {
                delta_frame_->push(std::make_unique<DeletionDelta>(block, map_));
            }
        }
    }
    if (!live_blocks.empty() && delta_frame_) {
        delta_frame_->push(std::make_unique<BatchMotionDelta>(std::move(live_blocks), Point3{0,0,-layers_fallen_}, map_));
    }
}

void MoveProcessor::settle(FallComponent* comp) {
    comp->settled_ = true;
    handle_unique_blocks(comp);
    for (FallComponent* above : comp->above_) {
        if (!above->settled_) {
            settle(comp);
        }
    }
}

