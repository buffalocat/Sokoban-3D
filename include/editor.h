#ifndef EDITOR_H
#define EDITOR_H

#include "common.h"

class Room;

class Editor {
public:
    Editor();
    //Internal state methods
    Point pos();
    void shift_pos(Point d);
    void set_pos(Point p);
    void clamp_pos(int width, int height);
    Room* room();
    void set_room(Room* room);

    //GUI drawing methods
    void ShowMainWindow(bool* p_open);

private:
    Point pos_;
    Room* room_;
};

#endif // EDITOR_H
