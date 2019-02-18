#include "mainmenustate.h"

#include <memory>
#include "editorstate.h"

MainMenuState::MainMenuState(): GameState() {}

MainMenuState::~MainMenuState() {}

void MainMenuState::main_loop() {
    check_for_quit();
    create_child(std::make_unique<EditorState>(gfx_));
}
