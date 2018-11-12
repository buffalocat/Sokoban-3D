#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include "common.h"
#include "gamestate.h"

class Room;

class EditorState;

struct EditorRoom {
    std::unique_ptr<Room> room;
    Point start_pos;
    Point cam_pos;
    bool changed;
    EditorRoom(std::unique_ptr<Room>, Point);
};

class EditorTab {
public:
    EditorTab(EditorState*, GraphicsManager*);
    virtual ~EditorTab() = default;
    virtual void main_loop(EditorRoom*) = 0;
    virtual void handle_left_click(EditorRoom*, Point);
    virtual void handle_right_click(EditorRoom*, Point);

protected:
    EditorState* editor_;
    GraphicsManager* gfx_;
};

/*
class Camera;

class CameraTab: public EditorTab {
public:
    CameraTab(RoomManager*);
    ~CameraTab();
    void draw();
    void handle_left_click(Point);
    void handle_right_click(Point);

private:
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

//*/

class EditorState: public GameState {
public:
    EditorState(GraphicsManager*);
    void main_loop();

    void set_active_room(std::string name);
    int get_room_names(const char* room_names[]);

    void new_room(std::string name, int w, int h);
    bool load_room(std::string name);
    void save_room(EditorRoom* eroom, bool commit);
    void unload_current_room();
    void commit_current_room();
    void commit_all();

    void begin_test();

private:
    std::map<std::string, std::unique_ptr<EditorRoom>> rooms_;
    EditorRoom* active_room_;

    std::map<std::string, std::unique_ptr<EditorTab>> tabs_;
    EditorTab* active_tab_;

    bool ortho_cam_;
    int keyboard_cooldown_;

    void handle_mouse_input();
    void handle_keyboard_input();

    bool want_capture_keyboard();
    bool want_capture_mouse();
    Point get_pos_from_mouse();
    void clamp_to_active_map(Point&);
};

#endif // EDITORSTATE_H
