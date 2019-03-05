#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H


#include "gamestate.h"

class MainMenuState: public GameState {
public:
    MainMenuState();
    ~MainMenuState();
    void main_loop();
};

#endif // MAINMENUSTATE_H
