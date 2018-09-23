#ifndef EDITOR_H
#define EDITOR_H

#include "common.h"

enum class EditorMode {
    SaveLoad,
    Objects,
    Camera,
};

class Room;
class GameObject;

class Editor {
public:
    Editor(GLFWwindow*);
    //Internal state methods
    Point pos();
    void shift_pos(Point d);
    void set_pos(Point p);
    void clamp_pos(int width, int height);
    void set_room(Room* room);

    void handle_input();

    void draw_saveload_tab();
    void draw_objects_tab();
    void draw_camera_tab();

    std::unique_ptr<GameObject> create_obj(Point pos);

    //GUI drawing methods
    void ShowMainWindow(bool* p_open);

private:
    GLFWwindow* window_;
    Room* room_;
    Point pos_;

    EditorMode mode_;

    //State variables for creation options, etc.
    int solid_obj;
    int pb_sticky;
    bool is_car;
    int sb_ends;
};

#endif // EDITOR_H
