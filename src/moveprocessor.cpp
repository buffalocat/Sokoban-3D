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
push_comps_ {}, push_comps_unique_ {},
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
    push_comps_.clear();
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
    collect_moving_and_weak_links(push_comps_[block], weak_links);
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
    if (PushComponent* comp = push_comps_[start_block]) {
        return !comp->blocked_;
    }
    auto comp_unique = std::make_unique<PushComponent>();
    PushComponent* comp = comp_unique.get();
    push_comps_unique_.push_back(std::move(comp_unique));
    start_block->collect_strong_component(map_, comp, dir_, push_comps_);
    for (auto block : comp->blocks_) {
        push_comps_[block] = comp;
    }
    for (auto block : comp->blocks_) {
        if (!block->pushable_) {
            comp->blocked_ = true;
            break;
        }
        if (GameObject* in_front = map_->view(block->pos_ + dir_)) {
            if (in_front->pushable_ && compute_push_component(in_front)) {
                comp->pushing_.push_back(push_comps_[in_front]);
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
    map_->alert_activated_listeners(delta_frame_);
    map_->check_signalers(delta_frame_, fall_check_);
}

void MoveProcessor::begin_fall_cycle() {
    while (!fall_check_.empty()) {
        fall_step();
        fall_comps_.clear();
        fall_comps_unique_.clear();
        fall_check_.clear();
        map_->check_signalers(delta_frame_, fall_check_);
    }
}

void MoveProcessor::fall_step() {
    while (!fall_check_.empty()) {
        std::vector<GameObject*> next_check {};
        for (GameObject* block : fall_check_) {
            if (fall_comps_.count(block)) {
                //fall_comps_.push_back(block->make_weak_component(map_));
                //block->w_comp()->collect_above(next_check, map_);
            }
        }
        fall_check_ = std::move(next_check);
    }
    // Initial check for land
    check_land_first();
    for (auto& comp : fall_comps_) {
        comp->collect_falling_unique(map_);
        comp->reset_blocks_comps();
    }
    int layers_fallen = 0;
    while (true) {
        ++layers_fallen;
        bool done_falling = true;
        for (auto& comp : fall_comps_) {
            if (comp->drop_check(layers_fallen, map_, delta_frame_)) {
                done_falling = false;
            }
        }
        if (done_falling) {
            break;
        }
        for (auto& comp : fall_comps_) {
            if (comp->falling()) {
                comp->check_land_sticky(layers_fallen, map_, delta_frame_);
            }
        }
    }
}

void MoveProcessor::check_land_first() {
    for (auto& comp : fall_comps_) {
        comp->check_land_first(map_);
    }
    for (auto& comp : fall_comps_) {
        comp->reset_blocks_comps();
    }
}
