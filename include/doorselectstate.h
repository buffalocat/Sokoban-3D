#ifndef DOORSELECTSTATE_H
#define DOORSELECTSTATE_H


#include "editorbasestate.h"

class Room;
class Door;

class DoorSelectState: public EditorBaseState {
public:
    DoorSelectState(Room*, Point3 cam_pos, Point3* exit_pos);
    ~DoorSelectState();
    void main_loop();

private:
    Room* room_;
    Point3 cam_pos_;
    Point3* exit_pos_;

    virtual void handle_left_click(Point3);
    virtual void handle_right_click(Point3);
};

#endif // DOORSELECTSTATE_H
