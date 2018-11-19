#ifndef OBJECTTAB_H
#define OBJECTTAB_H

#include "common.h"
#include "editortab.h"

class ObjectTab: public EditorTab {
public:
    ObjectTab(EditorState*, GraphicsManager*);
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point);
    void handle_right_click(EditorRoom*, Point);

private:
    int layer;
    int obj_code;

    int color;
    int pb_sticky;
    bool is_car;
    int sb_ends;
    bool persistent;
    bool default_state;
};

#endif // OBJECTTAB_H
