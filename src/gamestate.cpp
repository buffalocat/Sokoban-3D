#include "gamestate.h"

GameState::GameState(): parent_ {}, current_state_ptr_ {} {}

GameState::~GameState() {}

void GameState::create_child(std::unique_ptr<GameState> child) {
    child->parent_ = std::move(*current_state_ptr_);
    child->current_state_ptr_ = current_state_ptr_;
    child->gfx_ = gfx_;
    *current_state_ptr_ = std::move(child);
}

void GameState::defer_to_parent() {
    *current_state_ptr_ = std::move(parent_);
}

void GameState::set_csp(std::unique_ptr<GameState>* csp) {
    current_state_ptr_ = csp;
}
