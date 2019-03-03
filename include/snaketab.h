#ifndef SNAKETAB_H
#define SNAKETAB_H

#include "editortab.h"

class SnakeTab: public EditorTab {
public:
    SnakeTab(EditorState*, GraphicsManager*);
    ~SnakeTab();

    void init();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);
    void handle_right_click(EditorRoom*, Point3);

    void handle_click_generic(EditorRoom*, Point3);
};

#endif // SNAKETAB_H
