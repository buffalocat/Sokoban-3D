#ifndef ROOM_H
#define ROOM_H

#include <fstream>

#include "common.h"

#include "roommap.h"
#include "camera.h"
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

class Room {
public:

    Room(std::string, std::unique_ptr<RoomMap>, std::unique_ptr<Camera>);
    Room(int w, int h);
    RoomMap* room_map();
    Camera* camera();
    std::string name();
    void serialize(std::ofstream& file);
    void save();

    void set_default_player_pos(Point);
    Point default_player_pos();

private:
    std::string name_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;
    // For writing "shut" doors to the map before anything else!
    std::vector<Point> doors_;
    Point default_player_pos_;
};

class RoomManager {
public:
    RoomManager(GLFWwindow*, Shader* shader);
    bool init_load(std::string map_name);
    void init_make(int w, int h);
    std::unique_ptr<Room> load(std::string map_name, bool edit_mode, Point start={-1,-1});
    void save(std::string map_name, bool overwrite);

    void set_editor(Editor* editor);
    void set_cur_room(Room* room);
    void set_player(Player* player);

    void use_door(MapLocation*, DeltaFrame*);

    RoomMap* room_map();
    Camera* camera();
    Player* player();

    void main_loop(bool& editor_mode);
    void handle_input(DeltaFrame*);
    void draw(bool editor_mode);

    void handle_input_editor_mode();
    void draw_editor_mode();

    void create_obj(std::unique_ptr<GameObject>);
    void delete_obj(Point);

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

    // These really shouldn't be here!
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

// These aren't methods - we don't even want to *accidentally* access the
// "old" map or camera while initializing the new ones!
void read_objects(std::ifstream& file, RoomMap*, RoomManager*);
void read_camera_rects(std::ifstream& file, Camera*);
void read_snake_link(std::ifstream& file, RoomMap*);
void read_door_dest(std::ifstream& file, RoomMap*);

#endif // ROOM_H
