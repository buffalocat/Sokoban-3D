#ifndef EDITOR_H
#define EDITOR_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Winline"
#pragma GCC diagnostic ignored "-Wpedantic"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#pragma GCC diagnostic pop

#include "roommap.h"
#include "common.h"


class Editor {
public:
    Editor(GLFWwindow*);
    void handle_input(DeltaFrame*, RoomMap*, Point cam_pos);
    void get_pos(RoomMap*, Point cam_pos);
    void create_obj(DeltaFrame*, RoomMap*);
    void destroy_obj(DeltaFrame*, RoomMap*);

private:
    GLFWwindow* window_;
    bool valid_pos_;
    Point pos_;
    int type_;
};

#endif // EDITOR_H
