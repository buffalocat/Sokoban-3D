#include "moveprocessor.h"
#include "block.h"
#include "worldmap.h"

MoveProcessor::MoveProcessor(WorldMap* world_map, Point dir): map_ {world_map}, dir_ {dir}, comps_ {} {}

void MoveProcessor::try_move(DeltaFrame* delta_frame) {
    std::vector<Component*> roots {};
    //NOTE: More checks will happen when Player is a separate layer in practice.
    for (Block* block : map_->movers()) {
        // NOTE: Later there should be special code for when block is a SnakeBlock*!
        roots.push_back(move_component(block, false));
    }
    for (Component* comp : roots) {
        comp->resolve_contingent();
    }
    // This section will contain some Block-type specific code.
    // I don't think this can be helped; for now we'll do it the dumb way.
    for (auto& p : comps_) {
        if (p.second->good()) {
            // Universal block movement code
            Block* obj = p.first;
            auto obj_unique = map_->take_quiet_id(obj->pos(), Layer::Solid, obj);
            obj->shift_pos(dir_, delta_frame);
            map_->put_quiet(std::move(obj_unique));
            // Special checks go here
            SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj);
            if (sb && sb->distance() = )
        }
    }
    // PULL SNAKES HERE!

    // The second iteration is unavoidable; it's not obvious whether it's better to
    // naively iterate through the whole set or to record the moved-set during the above
    for (auto& p : comps_) {
        if (p.second->good()) {
            // Also, reset any stray pushed snake data
            p.first->check_add_local_links(map_, delta_frame);
        }
    }
    for (auto& block : maybe_broken_weak_) {
        block->check_remove_local_links(map_, delta_frame);
    }
}

/** Attempt to move the strong component containing a block
 * In certain circumstances, we want to check a component even if
 * we have already seen it before; this is represented by recheck.
 * The return value is the component, whose state indicates success/failure.
 */
Component* MoveProcessor::move_component(Block* block, bool recheck) {
    if (!comps_.count(block)) {
        find_strong_component(block);
    }
    Component* comp = comps_[block].get();
    if (comp->check() || recheck) {
        comp->set_check(false);
        for (Block* cur : comp->blocks()) {
            try_push(comp, cur->shifted_pos(dir_));
            if (comp->bad()) {
                break;
            }
            for(auto& link : cur->get_weak_links()) {
                //NOTE: should it be necessary, false can be replaced with a new
                // method called Block::weak_recheck()
                auto moved_comp = move_component(link, false);
                comp->add_weak(moved_comp);
                if (moved_comp->bad()) {
                    maybe_broken_weak_.insert(link);
                }
            }
        }
    }
    return comp;
}

void MoveProcessor::try_push(Component* comp, Point pos) {
    if (!map_->valid(pos)) {
        comp->set_bad();
    } else {
        GameObject* obj = map_->view(pos, Layer::Solid);
        if (obj) {
            Block* block = dynamic_cast<Block*>(obj);
            if (block) {
                auto pushed_comp = move_component(block, block->push_recheck());
                if (pushed_comp->bad()) {
                    comp->set_bad();
                } else {
                    comp->add_push(pushed_comp);
                }
            } else {
                comp->set_bad();
            }
        }
    }
}

void MoveProcessor::find_strong_component(Block* block) {
    std::vector<Block*> to_check = {block};
    auto comp = std::make_shared<Component>();
    while (!to_check.empty()) {
        Block* cur = to_check.back();
        to_check.pop_back();
        comp->add_block(cur);
        comps_[cur] = comp;
        for (auto& link : cur->get_strong_links()) {
            if (!comps_.count(link)) {
                to_check.push_back(link);
            }
        }
    }
 }

Component::Component(): state_ {ComponentState::Contingent}, check_ {true}, blocks_ {}, push_ {}, weak_ {} {}

void Component::resolve_contingent() {
    if (state_ == ComponentState::Contingent) {
        state_ = ComponentState::Good;
        for (auto& comp : push_) {
            comp->resolve_contingent();
        }
        for (auto& comp : weak_) {
            comp->resolve_contingent();
        }
    }
}

bool Component::bad() {
    return state_ == ComponentState::Bad;
}

bool Component::good() {
    return state_ == ComponentState::Good;
}

void Component::set_bad() {
    state_ = ComponentState::Bad;
}

void Component::set_check(bool check) {
    check_ = check;
}

const std::vector<Block*>& Component::blocks() {
    return blocks_;
}

bool Component::check() {
    return check_;
}

void Component::add_block(Block* block) {
    blocks_.push_back(block);
}

void Component::add_push(Component* comp) {
    push_.push_back(comp);
}

void Component::add_weak(Component* comp) {
    weak_.push_back(comp);
}
