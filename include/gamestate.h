#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "common.h"

class GraphicsManager;

class GameState {
public:
    GameState();
    virtual ~GameState();
    void create_child(std::unique_ptr<GameState> child);
    void defer_to_parent();
    void set_csp(std::unique_ptr<GameState>*);
    virtual void main_loop() = 0;

protected:
    GraphicsManager* gfx_;

private:
    std::unique_ptr<GameState>* current_state_ptr_;
    std::unique_ptr<GameState> parent_;
};

class MainMenuState {};

#endif // GAMESTATE_H
