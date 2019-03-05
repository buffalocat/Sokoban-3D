#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <memory>

class GLFWwindow;
class GraphicsManager;

class GameState {
public:
    GameState();
    virtual ~GameState();
    void create_child(std::unique_ptr<GameState> child);
    void defer_to_parent();
    void set_graphics(GraphicsManager*);
    void set_csp(std::unique_ptr<GameState>*);
    virtual void main_loop() = 0;
    void check_for_quit();

    GraphicsManager* gfx_;
    GLFWwindow* window_;

private:
    std::unique_ptr<GameState> parent_;
    std::unique_ptr<GameState>* current_state_ptr_;
    bool can_quit_;
};

#endif // GAMESTATE_H
