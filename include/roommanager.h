/*

#ifndef ROOM_H
#define ROOM_H

#include <fstream>

#include "common.h"

#include "delta.h"
#include "gameobject.h"

class GLFWwindow;
class Shader;
class Editor;
class RoomMap;
class Camera;
class UndoStack;
class DeltaFrame;
class Block;
class Door;

struct MapLocation;

class RoomManager {
public:
    RoomManager(GLFWwindow*, Shader* shader);
    bool init_load(std::string map_name);
    void init_make(std::string map_name, int w, int h);
    Room* activate(std::string map_name, Point start={-1,-1});
    Room* load(std::string map_name, Point start={-1,-1});
    std::unique_ptr<Room> load_from_file(std::string& map_name, std::ifstream& file, Point start={-1,-1});
    void save(std::string map_name, bool overwrite, bool editor_mode);

    void set_editor(Editor* editor);
    void set_cur_room(std::string name, Player* player);
    void set_cur_room(Room* room);
    void set_player(Player* player);

    void use_door(MapLocation*, DeltaFrame*);

    Room* room();
    int get_room_names(const char* room_names[]);

    RoomMap* room_map();
    Camera* camera();
    Player* player();

    void main_loop(bool& editor_mode);
    void handle_input(DeltaFrame*);
    void draw(bool editor_mode);

    void handle_input_editor_mode();
    void draw_editor_mode();

    void create_obj(Layer layer, std::unique_ptr<GameObject>);
    void delete_obj(Point, Layer layer);

    void make_door(Point pos, Point dest, std::string room_name);

    Point get_pos_from_mouse();
    bool valid(Point);

private:
    GLFWwindow* window_;
    Shader* shader_;
    Editor* editor_;
    UndoStack undo_stack_;

    int cooldown_;
    Player* player_;

    std::unordered_map<std::string, std::unique_ptr<Room>> rooms_;
    Room* cur_room_;
    // For convenience
    RoomMap* cur_map_;
    Camera* cur_camera_;

};

#endif // ROOM_H
//*/
