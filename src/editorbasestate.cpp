#include "editorbasestate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wswitch-default"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#pragma GCC diagnostic pop


#include "common_constants.h"
#include "room.h"
#include "roommap.h"
#include "gameobject.h"


const std::unordered_map<int, Point3> EDITOR_MOVEMENT_KEYS {
    {GLFW_KEY_D, {1, 0, 0}},
    {GLFW_KEY_A, {-1,0, 0}},
    {GLFW_KEY_S, {0, 1, 0}},
    {GLFW_KEY_W, {0,-1, 0}},
    {GLFW_KEY_E, {0, 0, 1}},
    {GLFW_KEY_Q, {0, 0,-1}},
};


EditorBaseState::EditorBaseState(): GameState(),
ortho_cam_ {true}, one_layer_ {false}, keyboard_cooldown_ {0} {}

EditorBaseState::~EditorBaseState() {}

bool EditorBaseState::want_capture_keyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool EditorBaseState::want_capture_mouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

Point3 EditorBaseState::get_pos_from_mouse(Point3 cam_pos) {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    if (xpos >= 0 && xpos < SCREEN_WIDTH && ypos >= 0 && ypos < SCREEN_HEIGHT) {
        int x = ((int)xpos + MESH_SIZE*cam_pos.x - (SCREEN_WIDTH - MESH_SIZE) / 2) / MESH_SIZE;
        int y = ((int)ypos + MESH_SIZE*cam_pos.y - (SCREEN_HEIGHT - MESH_SIZE) / 2) / MESH_SIZE;
        return {x, y, cam_pos.z};
    }
    return {-1,-1,-1};
}

void EditorBaseState::display_hover_pos_object(Point3 cam_pos, RoomMap* room_map) {
    Point3 mouse_pos = get_pos_from_mouse(cam_pos);

    if (mouse_pos.x == -1) {
        ImGui::Text("Hover Pos: Out of Bounds");\
    } else {
        ImGui::Text("Hover Pos: (%d,%d,%d)", mouse_pos.x, mouse_pos.y, mouse_pos.z);
        if (GameObject* obj = room_map->view(mouse_pos)) {
            ImGui::Text(obj->to_str().c_str());
        } else {
            ImGui::Text("Empty");
        }
    }
}

void EditorBaseState::clamp_to_room(Point3& pos, Room* room) {
    RoomMap* cur_map = room->map();
    pos = {
        std::max(0, std::min(cur_map->width_ - 1, pos.x)),
        std::max(0, std::min(cur_map->height_ - 1, pos.y)),
        std::max(0, std::min(cur_map->depth_ - 1, pos.z))
    };
}

void EditorBaseState::handle_mouse_input(Point3 cam_pos, Room* room) {
    if (want_capture_mouse() || !ortho_cam_) {
        return;
    }
    Point3 mouse_pos = get_pos_from_mouse(cam_pos);
    if (!room->valid(mouse_pos)) {
        return;
    }
    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        handle_left_click(mouse_pos);
    } else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        handle_right_click(mouse_pos);
    }
}

void EditorBaseState::handle_keyboard_input(Point3& cam_pos, Room* room) {
    if (keyboard_cooldown_ > 0) {
        --keyboard_cooldown_;
        return;
    }
    if (want_capture_keyboard()) {
        return;
    }
    keyboard_cooldown_ = MAX_COOLDOWN;
    for (auto p : EDITOR_MOVEMENT_KEYS) {
        if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
            if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                cam_pos += FAST_MAP_MOVE * p.second;
            } else {
                cam_pos += p.second;
            }
            clamp_to_room(cam_pos, room);
            return;
        }
    }
    if (glfwGetKey(window_, GLFW_KEY_C)) {
        ortho_cam_ = !ortho_cam_;
        return;
    } else if (glfwGetKey(window_, GLFW_KEY_F)) {
        one_layer_ = !one_layer_;
        return;
    }
    keyboard_cooldown_ = 0;
}
