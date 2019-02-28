#include "horizontalstepprocessor.h"

#include "player.h"
#include "snakeblock.h"
#include "roommap.h"

HorizontalStepProcessor::HorizontalStepProcessor(RoomMap* room_map, DeltaFrame* delta_frame, Point3 dir,
    std::vector<GameObject*>& fall_check, std::vector<GameObject*>& moving_blocks):
push_comps_unique_ {},
link_break_check_ {}, moving_snakes_ {},
snakes_to_reset_ {}, snakes_to_recheck_ {},
fall_check_ {fall_check}, moving_blocks_ {moving_blocks},
map_ {room_map}, delta_frame_ {delta_frame}, dir_ {dir} {}

HorizontalStepProcessor::~HorizontalStepProcessor() {}


void HorizontalStepProcessor::run(Player* player) {
    compute_push_component_tree(player);
    if (!moving_blocks_.empty()) {
        perform_horizontal_step();
    }
}


// Try to push the block and build the resulting component tree
// Return whether block is able to move
bool HorizontalStepProcessor::compute_push_component_tree(GameObject* block) {
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
bool HorizontalStepProcessor::compute_push_component(GameObject* start_block) {
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


void HorizontalStepProcessor::collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links) {
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

void HorizontalStepProcessor::perform_horizontal_step() {
    // Any block which moved forward could have moved off a ledge
    fall_check_ = moving_blocks_;
    // Keep a list of snakes which did not move but may have gained links anyway
    std::unordered_set<SnakeBlock*> link_add_check {};
    link_add_check.insert(moving_snakes_.begin(), moving_snakes_.end());
    for (auto sb : link_break_check_) {
        fall_check_.push_back(sb);
        // NOTE: May be made redundant by the link_add_check insertion for all moving_snakes_!
        link_add_check.insert(sb);
        sb->remove_moving_links(delta_frame_);
    }
    SnakePuller snake_puller {map_, delta_frame_, moving_blocks_, link_add_check, fall_check_};
    for (auto sb : moving_snakes_) {
        sb->collect_maybe_confused_neighbors(map_, link_add_check);
        snake_puller.prepare_pull(sb);
    }
    // MAP BECOMES INCONSISTENT HERE (potential ID overlap)
    // In this section of code, the map can't be viewed
    auto forward_moving_blocks = moving_blocks_;
    snake_puller.perform_pulls();
    map_->batch_shift(std::move(forward_moving_blocks), dir_, delta_frame_);
    // MAP BECOMES CONSISTENT AGAIN HERE
    for (auto sb : moving_snakes_) {
        sb->reset_distance_and_target();
    }
    for (auto sb : link_add_check) {
        sb->check_add_local_links(map_, delta_frame_);
    }
}
