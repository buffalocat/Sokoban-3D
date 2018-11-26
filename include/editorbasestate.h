#ifndef EDITORBASESTATE_H
#define EDITORBASESTATE_H

#include "common.h"
#include "gamestate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <dear/imgui.h>
#pragma GCC diagnostic pop

class Room;

class EditorBaseState: public GameState {
public:
    EditorBaseState();
    virtual ~EditorBaseState();

protected:
    bool ortho_cam_;
    int keyboard_cooldown_;

    bool want_capture_keyboard();
    bool want_capture_mouse();

    Point get_pos_from_mouse(Point cam_pos);
    void handle_mouse_input(Point, Room*);
    void handle_keyboard_input(Point3&, Room*);
    void clamp_to_room(Point3&, Room*);

    virtual void handle_left_click(Point) = 0;
    virtual void handle_right_click(Point) = 0;
};

#endif // EDITORBASESTATE_H
