#include "moveprocessor.h"
#include "block.h"
#include "snakeblock.h"
#include "player.h"
#include "delta.h"
#include "roommap.h"
#include "switch.h"
#include "component.h"

MoveProcessor::MoveProcessor(Player* player, RoomMap* room_map, Point3 dir, DeltaFrame* delta_frame):
player_ {player}, map_ {room_map}, delta_frame_ {delta_frame}, dir_ {dir},
move_comps_ {}, below_release_ {}, below_press_ {},
moving_blocks_ {}, fall_check_ {}, link_add_check_ {}, link_break_check_ {}, fall_comps_ {},
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
    Block* car = static_cast<Block*>(map_->view(player_->shifted_pos({0,0,-1})));
    Block* adj = dynamic_cast<Block*>(map_->view(car->shifted_pos(dir_)));
    if (adj && car->color() == adj->color()) {
        auto player_unique = map_->take_quiet(player_);
        player_->set_linear_animation(dir_);
        if (delta_frame_) {
            delta_frame_->push(std::make_unique<MotionDelta>(std::vector<std::pair<GameObject*, Point3>> {std::make_pair(player_, player_->pos())}, map_));
        }
        moving_blocks_.push_back(player_);
        player_->shift_pos_from_animation();
        map_->put_quiet(std::move(player_unique));
    }
}

void MoveProcessor::move_general() {
    init_movement_components();
    move_components();
}

bool MoveProcessor::update() {
    if (--frames_ == 0) {
        perform_switch_checks();
        begin_fall_cycle();
        return true;
    } else {
        for (Block* block : moving_blocks_) {
            block->update_animation();
        }
    }
    return false;
}

void MoveProcessor::abort() {
    for (Block* block : moving_blocks_) {
        block->reset_animation();
    }
}

void MoveProcessor::color_change_check() {
    Block* car = player_->get_car(map_, false);
    if (!(car && car->cycle_color(false))) {
        return;
    }
    if (delta_frame_) {
        delta_frame_->push(std::make_unique<ColorChangeDelta>(car));
    }
    fall_check_.push_back(car);
    for (Point3 d : DIRECTIONS) {
        Block* block = dynamic_cast<Block*>(map_->view(car->shifted_pos(d)));
        if (block) {
            fall_check_.push_back(block);
        }
    }
    begin_fall_cycle();
}

// Return whether all blocks in the group can move
bool MoveProcessor::find_push_component_tree(std::vector<Block*> block_group) {
    std::unordered_set<Block*> weak_links {};
    // TODO : change false to an appropriate conditional
    // because a snakeblock "pretends" to be pushed in certain situations
    for (Block* block : block_group) {
        if (!push_component_tree_helper(block, weak_links, false)) {
            return false;
        }
    }
    moving_comps_.push_back(push_comps_[block]);
    for (Block* link : weak_links) {
        // TODO : use result of this to determine broken snake links
        find_push_component_tree(link);
    }
    return true;
}

// Return whether the block can move
bool MoveProcessor::push_component_tree_helper(Block* block, std::unordered_set<Block*>& weak_links, bool pushed) {
    if (PushComponent* comp = push_comps_[block]) {
        if (pushed && !comp->pushed_) {
            comp->pushed_ = true;
            // If this is a snake being pushed for the first time, check links!
            if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
                for (SnakeBlock* link : sb->links_) {
                    weak_links.insert(link);
                }
            }
        }
        return !comp->blocked_;
    }
    auto comp_unique = std::make_unique<PushComponent>(pushed));
    PushComponent* comp = comp_unique.get();
    push_comps_unique_.push_back(std::move(comp_unique));
    if (block->sticky_) {
        // Form the strong component, collecting weak links
        Sticky strong_sticky_condition = block->sticky_ & Sticky::STRONGSTICK;
        Sticky weak_sticky_condition = block->sticky_ & Sticky::WEAKSTICK;
        std::vector<Block*> to_check {block};
        unsigned char color = block->color();
        while (!to_check.empty()) {
            Block* cur = to_check.back();
            to_check.pop_back();
            push_comps_[cur] = comp;
            comp->blocks_.push_back(cur);
            for (Point3 d : DIRECTIONS) {
                Block* adj = dynamic_cast<Block*>(room_map->view(cur->pos_ + d));
                if (adj && push_comps_.count(adj) == 0 && adj->color() == color) {
                    if (adj->sticky_ & strong_sticky_condition) {
                        to_check.push_back(adj);
                    // Blocks in front don't become weak links
                    } else if (adj->sticky_ & weak_sticky_condition && d != dir_) {
                        weak_links.insert(adj);
                    }
                }
            }
        }
    } else {
        push_comps_[cur] = comp;
        comp->blocks_.push_back(block);
        if (pushed && SnakeBlock* sb = dynamic_cast<SnakeBlock*>(block)) {
            for (SnakeBlock* link : sb->links_) {
                weak_links.insert(link);
            }
        }
    }
    for (Block* block : comp->blocks_) {
        if (!block->pushable()) {
            comp->blocked_ = true;
            break;
        }
        if (GameObject* in_front = room_map->view(cur->pos_ + dir_)) {
            if (Block* block_in_front = dynamic_cast<Block*>(in_front)) {
                if (push_component_tree_helper(block_in_front, weak_links, true)) {
                    comp->pushing_.insert(push_comps_[block_in_front]);
                } else {
                    // The Block we tried to push couldn't move
                    comp->blocked_ = true;
                    break;
                }
            } else {
                // The thing we tried to push wasn't a Block
                comp->blocked_ = true;
                break;
            }
        }
    }
    return !comp->blocked_;
}


void MoveProcessor::init_movement_components() {
    /*
    std::vector<StrongComponent*> roots {};
    std::vector<std::pair<StrongComponent*, StrongComponent*>> dependent_pairs {};
    make_root(player_, roots);
    if (player_->state() == RidingState::Riding) {
        Block* car = player_->get_car(map_, true);
        make_root(car, roots);
        dependent_pairs.push_back(std::make_pair(player_->s_comp(), car->s_comp()));
    }
    // When relevant, create other root components
    for (StrongComponent* comp : roots) {
        try_move_component(comp);
    }
    for (auto& p : dependent_pairs) {
        if (p.first->bad() || p.second->bad()) {
            p.first->set_bad();
            p.second->set_bad();
        }
    }
    for (StrongComponent* comp : roots) {
        comp->resolve_contingent();
    }
    */
}

void MoveProcessor::move_components() {
    std::vector<std::pair<std::unique_ptr<SnakeBlock>, SnakeBlock*>> split_snakes {};
    SnakePuller snake_puller {map_, delta_frame_, link_add_check_, split_snakes, moving_blocks_};
    for (auto sb : link_break_check_) {
        sb->check_remove_local_links(delta_frame_);
    }
    for (auto& comp : move_comps_) {
        if (comp->good()) {
            comp->collect_blocks(moving_blocks_, dir_);
            auto snake_comp = dynamic_cast<SnakeComponent*>(comp.get());
            if (snake_comp) {
                link_add_check_.push_back(snake_comp->block());
                if (!snake_comp->pushed()) {
                    snake_puller.prepare_pull(snake_comp->block());
                }
            }
        }
    }
    std::vector<std::unique_ptr<GameObject>> move_unique {};
    for (auto block : moving_blocks_) {
        move_unique.push_back(map_->take_quiet(block));
        fall_check_.push_back(block);
        Block* above = dynamic_cast<Block*>(map_->view(block->shifted_pos({0,0,1})));
    }
    std::vector<std::pair<GameObject*, Point3>> moved_blocks {};
    for (auto& obj : move_unique) {
        Block* block = static_cast<Block*>(obj.get());
        moved_blocks.push_back(std::make_pair(block, block->pos()));
        block->shift_pos_from_animation();
        map_->put_quiet(std::move(obj));
    }
    if (!moved_blocks.empty() && delta_frame_) {
        delta_frame_->push(std::make_unique<MotionDelta>(std::move(moved_blocks), map_));
    }
    for (auto& p : split_snakes) {
        SnakeBlock* sb = p.first.get();
        map_->put(std::move(p.first), delta_frame_);
        moving_blocks_.push_back(sb);
        sb->add_link(p.second, delta_frame_);
    }
    for (auto& comp : move_comps_) {
        comp->reset_blocks_comps();
    }
    move_comps_.clear();
}

void MoveProcessor::perform_switch_checks() {
    map_->alert_activated_listeners();
    map_->check_signalers(delta_frame_, &fall_check_);
}

bool MoveProcessor::try_move_component(StrongComponent* comp) {
    for (Point3 pos : comp->to_push(dir_)) {
        if (!try_push(comp, pos)) {
            comp->set_bad();
            return false;
        }
    }
    for(Block* link : comp->get_weak_links(map_)) {
        if (!link->s_comp()) {
            auto link_comp = link->make_strong_component(map_);
            try_move_component(link_comp.get());
            move_comps_.push_back(std::move(link_comp));
        }
        if (!link->s_comp()->bad()) {
            comp->add_weak(link->s_comp());
        } else {
            fall_check_.push_back(link);
            auto sb = dynamic_cast<SnakeBlock*>(link);
            if (sb) {
                link_break_check_.push_back(sb);
            }
        }
    }
    return true;
}

bool MoveProcessor::try_push(StrongComponent* comp, Point3 pos) {
    if (!map_->valid(pos)) {
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
    StrongComponent* pushed_comp = block->s_comp();
    if (pushed_comp) {
        if (!pushed_comp->push_recheck()) {
            return !pushed_comp->bad();
        }
    } else {
        auto unique_comp = block->make_strong_component(map_);
        pushed_comp = unique_comp.get();
        pushed_comp->set_pushed();
        move_comps_.push_back(std::move(unique_comp));
    }
    comp->add_push(pushed_comp);
    return try_move_component(pushed_comp);
}

void MoveProcessor::begin_fall_cycle() {
    while (!fall_check_.empty()) {
        fall_step();
        map_->check_signalers(delta_frame_, &fall_check_);
    }
}

void MoveProcessor::fall_step() {
    while (!fall_check_.empty()) {
        std::vector<Block*> next_check {};
        for (Block* block : fall_check_) {
            if (!block->w_comp()) {
                fall_comps_.push_back(block->make_weak_component(map_));
                block->w_comp()->collect_above(next_check, map_);
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
    fall_comps_.clear();
    fall_check_.clear();
}

void MoveProcessor::check_land_first() {
    for (auto& comp : fall_comps_) {
        comp->check_land_first(map_);
    }
    for (auto& comp : fall_comps_) {
        comp->reset_blocks_comps();
    }
}
