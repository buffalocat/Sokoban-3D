#ifndef EDITORSTATE_H
#define EDITORSTATE_H

#include "common.h"
#include "editorbasestate.h"
#include "editortab.h"
#include "room.h"

struct EditorRoom {
    std::unique_ptr<Room> room;
    Point3 start_pos;
    Point3 cam_pos;
    bool changed;
    EditorRoom(std::unique_ptr<Room>, Point3);
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
//*/

class EditorState: public EditorBaseState {
public:
    EditorState(GraphicsManager* gfx);
    ~EditorState() = default;
    void main_loop();

    void set_active_room(std::string name);
    int get_room_names(const char* room_names[]);
    EditorRoom* get_room(std::string name);

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

    void handle_left_click(Point);
    void handle_right_click(Point);
};

#endif // EDITORSTATE_H
