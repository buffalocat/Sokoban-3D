#include "gamestate.h"

#include "graphicsmanager.h"
#include "editorstate.h"
#include "room.h"
#include "roommap.h"
#include "camera.h"
#include "gameobject.h"

GameState::GameState(GraphicsManager* gfx):
gfx_ {gfx}, window_ {gfx->window()},
parent_ {}, current_state_ptr_ {},
can_quit_ {true} {}

void GameState::create_child(std::unique_ptr<GameState> child) {
    child->parent_ = std::move(*current_state_ptr_);
    child->current_state_ptr_ = current_state_ptr_;
    *current_state_ptr_ = std::move(child);
}

void GameState::defer_to_parent() {
    *current_state_ptr_ = std::move(parent_);
}

void GameState::set_csp(std::unique_ptr<GameState>* csp) {
    current_state_ptr_ = csp;
}

void GameState::check_for_quit() {
    if (can_quit_ && glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (parent_) {
            parent_->can_quit_ = false;
        }
        defer_to_parent();
    } else if (!can_quit_ && glfwGetKey(window_, GLFW_KEY_ESCAPE) != GLFW_PRESS) {
        can_quit_ = true;
    }
}

MainMenuState::MainMenuState(GraphicsManager* gfx): GameState(gfx) {}

void MainMenuState::main_loop() {
    check_for_quit();
    std::cout << "Running the \'main menu loop\'" << std::endl;
    create_child(std::make_unique<EditorState>(gfx_));
}
