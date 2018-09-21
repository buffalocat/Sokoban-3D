#ifndef EDITOR_H
#define EDITOR_H

#include "common.h"

class Room;
class GameObject;

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

    std::unique_ptr<GameObject> create_obj(Point pos);

    //GUI drawing methods
    void ShowMainWindow(bool* p_open);

private:
    Point pos_;
    Room* room_;

    //State variables for creation options, etc.
    int solid_obj;
    int pb_sticky;
    bool is_car;
    int sb_ends;
};

#endif // EDITOR_H
