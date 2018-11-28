#include "editorbasestate.h"

#include "room.h"
#include "roommap.h"

EditorBaseState::EditorBaseState(): GameState(),
ortho_cam_ {true}, keyboard_cooldown_ {0} {}

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

void EditorBaseState::clamp_to_room(Point3& pos, Room* room) {
    RoomMap* cur_map = room->room_map();
    pos = {
        std::max(0, std::min(cur_map->width() - 1, pos.x)),
        std::max(0, std::min(cur_map->height() - 1, pos.y)),
        std::max(0, std::min(cur_map->depth() - 1, pos.z))
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
    }
    keyboard_cooldown_ = 0;
}