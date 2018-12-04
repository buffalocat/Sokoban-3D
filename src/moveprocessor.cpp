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
move_comps_ {}, fall_check_ {}, link_break_check_ {}, fall_comps_ {} {}

MoveProcessor::~MoveProcessor() {}

void MoveProcessor::try_move() {
    if (player_->state() == RidingState::Bound) {
        move_bound();
    } else {
        move_general();
    }
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
        if (delta_frame_) {
            delta_frame_->push(std::make_unique<MotionDelta>(std::vector<Block*> {player_}, dir_, map_));
        }
        player_->shift_pos(dir_);
        map_->put_quiet(std::move(player_unique));
    }
}

void MoveProcessor::move_general() {
    init_movement_components();
    move_components();
    // Update snake links, do switch/other checks
    // Wait until the animation finishes
    begin_fall_cycle();
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

void MoveProcessor::init_movement_components() {
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
}

void MoveProcessor::make_root(Block* obj, std::vector<StrongComponent*>& roots) {
    auto component = obj->make_strong_component(map_);
    roots.push_back(component.get());
    move_comps_.push_back(std::move(component));
}

void MoveProcessor::move_components() {
    std::vector<Block*> block_move {};
    std::vector<SnakeBlock*> pull_snakes {};
    std::vector<SnakeBlock*> check_snakes {};
    for (auto& comp : move_comps_) {
        comp->collect_good(block_move);
        auto snake_comp = dynamic_cast<SnakeComponent*>(comp.get());
        if (snake_comp) {
            check_snakes.push_back(snake_comp->block());
            if (!snake_comp->pushed()) {
                //std::cout << "A Pull Snake at " << snake_comp->block()->pos() << std::endl;
                pull_snakes.push_back(snake_comp->block());
            }
        }
    }
    std::vector<std::unique_ptr<GameObject>> move_unique {};
    for (auto block : block_move) {
        move_unique.push_back(map_->take_quiet(block));
        fall_check_.push_back(block);
        Block* above = dynamic_cast<Block*>(map_->view(block->shifted_pos({0,0,1})));
        if (above && !above->s_comp()) {
            fall_check_.push_back(above);
        }
    }
    std::vector<GameObject*> below_release {};
    std::vector<GameObject*> below_press {};
    std::vector<Block*> moved_blocks {};
    GameObject* below;
    for (auto& block : move_unique) {
        below = map_->view(block->shifted_pos({0,0,-1}));
        if (below) {
            below_release.push_back(below);
        }
        //std::cout << "Shifting block " << block.get() << " at " << block->pos() << std::endl;
        moved_blocks.push_back(static_cast<Block*>(block.get()));
        block->shift_pos(dir_);
        //std::cout << "Shifted block to " << block->pos() << std::endl;
        below = map_->view(block->shifted_pos({0,0,-1}));
        if (below) {
            below_press.push_back(below);
        }
        map_->put_quiet(std::move(block));
    }
    if (!moved_blocks.empty() && delta_frame_) {
        delta_frame_->push(std::make_unique<MotionDelta>(std::move(moved_blocks), dir_, map_));
    }
    for (auto sb : link_break_check_) {
        sb->check_remove_local_links(delta_frame_);
    }
    SnakePuller snake_puller {map_, delta_frame_, dir_, check_snakes, below_release};
    //std::cout << "made snake puller\n" << std::endl;
    for (SnakeBlock* snake : pull_snakes) {
        snake_puller.prepare_pull(snake);
    }
    for (SnakeBlock* snake : pull_snakes) {
        snake->reset_target();
    }
    //std::cout << "finished pulling" << std::endl;
    for (auto& comp : move_comps_) {
        comp->reset_blocks_comps();
    }
    move_comps_.clear();
    for (auto obj : below_press) {
        obj->check_above_occupied(map_, delta_frame_);
    }
    for (auto obj : below_release) {
        obj->check_above_vacant(map_, delta_frame_);
    }
    for (auto sb : check_snakes) {
        sb->check_add_local_links(map_, delta_frame_);
    }
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
        std::cout << "block at " << block->pos() << " was pushed" << std::endl;
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
