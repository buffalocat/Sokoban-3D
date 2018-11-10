#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include "common.h"

#include "gamestate.h"

class Room;

struct EditorRoom;
class EditorState;

typedef std::map<std::string, std::unique_ptr<EditorRoom>> EditorRoomMap;


struct EditorRoom {
    Room* room;
    Point start_pos;
    Point cam_pos;
    bool changed;
};


/*
class EditorRoomSet: public RoomSet {
public:
    EditorRoomSet();
    bool activate(std::string name);
    bool load_fresh(std::string name);
    void save_all(bool commit);
    void set_changed(std::string name);

private:
    Room* active_room_;
    std::set<std::string, bool> changed_;
}; //*/

class EditorTab {
public:
    EditorTab(EditorState*, GraphicsManager*);
    virtual ~EditorTab();
    virtual void main_loop(EditorRoom*) = 0;
    //virtual void handle_left_click(Point) = 0;
    //virtual void handle_right_click(Point) = 0;

protected:
    EditorState* editor_;
    GraphicsManager* gfx_;
};


class SaveLoadTab: public EditorTab {
public:
    SaveLoadTab(EditorState*, GraphicsManager*);
    void main_loop(EditorRoom*);
};


class ObjectTab: public EditorTab {
public:
    ObjectTab(EditorState*, GraphicsManager*);
    void main_loop(EditorRoom*);
    //void handle_left_click(Point);
    //void handle_right_click(Point);

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
    EditorRoomMap* rooms();

private:
    EditorRoomMap rooms_;
    EditorRoom* active_room_;

    std::map<std::string, std::unique_ptr<EditorTab>> tabs_;
    EditorTab* active_tab_;

    GraphicsManager* gfx_;
    GLFWwindow* window_;
    bool ortho_cam_;
};

#endif // EDITORSTATE_H
