#include "mainmenustate.h"

#include "editorstate.h"

MainMenuState::MainMenuState(): GameState() {}

MainMenuState::~MainMenuState() {}

void MainMenuState::main_loop() {
    check_for_quit();
    std::cout << "Running the \'main menu loop\'" << std::endl;
    create_child(std::make_unique<EditorState>(gfx_));
}
