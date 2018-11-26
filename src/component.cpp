#include "component.h"
#include "roommap.h"
#include "block.h"

Component::Component(): state_ {ComponentState::Contingent}, weak_ {} {}

Component::~Component() {}

void Component::add_weak(Component* comp) {
    weak_.push_back(comp);
    comp->weak_.push_back(this);
}

bool Component::good() {
    return state_ == ComponentState::Good;
}

bool Component::bad() {
    return state_ == ComponentState::Bad;
}

void Component::set_bad() {
    state_ = ComponentState::Bad;
}

void Component::add_block(Block*) {}

ComplexComponent::ComplexComponent(): Component(), blocks_ {}, push_ {} {}

ComplexComponent::~ComplexComponent() {}

void ComplexComponent::resolve_contingent() {
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

void ComplexComponent::clean_up(std::vector<GameObject*>& to_move) {
    if (state_ == ComponentState::Good) {
        for (Block* block : blocks_) {
            block->reset_comp();
            to_move.push_back(block);
        }
    } else {
        for (Block* block : blocks_) {
            block->reset_comp();
        }
    }
}

void ComplexComponent::add_block(Block* block) {
    blocks_.push_back(block);
}

void ComplexComponent::add_push(Component* comp) {
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
Component(), block_ {block}, push_ {} {}

SingletonComponent::~SingletonComponent() {}

void SingletonComponent::add_push(Component* comp) {
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
    if (state_ == ComponentState::Contingent) {
        state_ = ComponentState::Good;
        if (push_) {
            push_->resolve_contingent();
        }
        for (auto& comp : weak_) {
            comp->resolve_contingent();
        }
    }
}

void SingletonComponent::clean_up(std::vector<GameObject*>& to_move) {
    if (state_ == ComponentState::Good) {
        to_move.push_back(block_);
    }
    block_->reset_comp();
}
