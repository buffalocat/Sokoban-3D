#ifndef EDITOR_H
#define EDITOR_H

#include "common.h"

class Room;
class GameObject;

class EditorTab {
public:
    EditorTab(Room* room);
    virtual ~EditorTab() = 0;
    virtual void draw() = 0;
    virtual void handle_left_click(Point) = 0;
    virtual void handle_right_click(Point) = 0;

protected:
    Room* room_;
};

class SaveLoadTab: public EditorTab {
public:
    SaveLoadTab(Room*);
    ~SaveLoadTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);
};

class ObjectTab: public EditorTab {
public:
    ObjectTab(Room*);
    ~ObjectTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);

private:
    int solid_obj;
    int pb_sticky;
    bool is_car;
    int sb_ends;
};

class CameraTab: public EditorTab {
public:
    CameraTab(Room*);
    ~CameraTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);
};

class Editor {
public:
    Editor(GLFWwindow*, Room*);
    //Internal state methods
    Point pos();
    void shift_pos(Point d);
    void set_pos(Point p);
    void clamp_pos(int width, int height);
    void set_room(Room* room);

    bool want_capture_keyboard();
    bool want_capture_mouse();

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

    // Editor Tabs!
    SaveLoadTab save_load_tab_;
    //friend class SaveLoadTab;
    ObjectTab object_tab_;
    //friend class ObjectTab;
    CameraTab camera_tab_;
    //friend class CameraTab;

    EditorTab* active_tab_;
};

#endif // EDITOR_H
