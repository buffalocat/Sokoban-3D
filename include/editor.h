#ifndef EDITOR_H
#define EDITOR_H

#include "common.h"

class RoomManager;
class GameObject;

class EditorTab {
public:
    EditorTab(RoomManager* room);
    virtual ~EditorTab() = 0;
    virtual void draw() = 0;
    virtual void handle_left_click(Point) = 0;
    virtual void handle_right_click(Point) = 0;

protected:
    RoomManager* mgr_;
};

class SaveLoadTab: public EditorTab {
public:
    SaveLoadTab(RoomManager*);
    ~SaveLoadTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);
};

class ObjectTab: public EditorTab {
public:
    ObjectTab(RoomManager*);
    ~ObjectTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);

private:
    int layer;
    int obj_code;

    int color;
    int pb_sticky;
    bool is_car;
    int sb_ends;
};

class Camera;

class CameraTab: public EditorTab {
public:
    CameraTab(RoomManager*);
    ~CameraTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);

private:
    Camera* camera_;
    int x1;
    int y1;
    int x2;
    int y2;
    float radius;
    int priority;
};

class DoorTab: public EditorTab {
public:
    DoorTab(RoomManager*);
    ~DoorTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);
};

class Editor {
public:
    Editor(GLFWwindow*, RoomManager*);
    //Internal state methods
    Point pos();
    void shift_pos(Point d);
    void set_pos(Point p);
    void clamp_pos(int width, int height);
    void set_room(RoomManager* room);

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
    RoomManager* mgr_;
    Point pos_;

    // Editor Tabs!
    SaveLoadTab save_load_tab_;
    ObjectTab object_tab_;
    CameraTab camera_tab_;
    DoorTab door_tab_;

    EditorTab* active_tab_;
};

#endif // EDITOR_H
