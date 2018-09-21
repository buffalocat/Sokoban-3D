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

class Room {
public:
    Room(GLFWwindow*, Shader* shader, Editor* editor, std::string map_name);
    Room(GLFWwindow*, Shader* shader, Editor* editor, int width, int height);
    // Initialization helpers
    void load(std::string map_name);
    void read_objects(std::ifstream& file);
    void save(std::string map_name);

    void main_loop(bool editor_mode);
    void handle_input(DeltaFrame*);
    void draw(bool editor_mode);

    void handle_input_editor_mode();
    void draw_editor_mode();

private:
    GLFWwindow* window_;
    Shader* shader_;
    Editor* editor_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<UndoStack> undo_stack_;
    int cooldown_;

    Block* player_;

    //These really shouldn't be here!
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

#endif // ROOM_H
