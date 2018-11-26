#ifndef DOORSELECTSTATE_H
#define DOORSELECTSTATE_H

#include "common.h"
#include "editorbasestate.h"

class Room;
class Door;

class DoorSelectState: public EditorBaseState {
public:
    DoorSelectState(Room*, Point3 cam_pos, Point3* door_pos, Door** door);
    virtual ~DoorSelectState();
    void main_loop();

private:
    Room* room_;
    Point3 cam_pos_;
    Point3* door_pos_;
    Door** door_;

    virtual void handle_left_click(Point);
    virtual void handle_right_click(Point);
};

#endif // DOORSELECTSTATE_H
