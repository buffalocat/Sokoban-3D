#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include <dear/imgui.h>

#pragma GCC diagnostic pop

#include "editor.h"
#include "room.h"

Editor::Editor(): pos_ {Point{0,0}}, room_ {} {}

Point Editor::pos() {
    return pos_;
}

void Editor::shift_pos(Point d) {
    pos_ = Point{pos_.x + d.x, pos_.y + d.y};
}

void Editor::set_pos(Point p) {
    pos_ = p;
}

void Editor::clamp_pos(int width, int height) {
    pos_ = Point {
        std::max(0, std::min(width-1, pos_.x)),
        std::max(0, std::min(height-1, pos_.y))
    };
}

Room* Editor::room() {
    std::cout << "room_ is " << room_ << std::endl;
    return room_;
}

void Editor::set_room(Room* room) {
    room_ = room;
    std::cout << "We just set room_ to " << room_ << std::endl;
}

void Editor::ShowMainWindow(bool* p_open) {
    if (!ImGui::Begin("My Editor Window", p_open, 0)) {
        ImGui::End();
        return;
    }

    if (ImGui::TreeNode("Save/Load")) {
        static char buf[32] = "";
        ImGui::InputText("map file", buf, IM_ARRAYSIZE(buf));
        if (ImGui::Button("Load Map")) {
            room_->load(buf);
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

/*
#include "gameobject.h"
#include "block.h"

Editor::Editor(GLFWwindow* window): window_ {window}, type_ {1}, pos_ {Point{0,0}}, valid_pos_ {false} {}

void Editor::handle_input(DeltaFrame* delta_frame, RoomMap* room_map, Point cam_pos) {
    for (int i = 0; i < 10; ++i) {
        if (glfwGetKey(window_, GLFW_KEY_0 + i) == GLFW_PRESS) {
            type_ = i;
            break;
        }
    }

    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        get_pos(room_map, cam_pos);
        if (!valid_pos_) {
            return;
        }
        create_obj(delta_frame, room_map);
    } else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        get_pos(room_map, cam_pos);
        if (!valid_pos_) {
            return;
        }
        destroy_obj(delta_frame, room_map);
    }
}

void Editor::get_pos(RoomMap* room_map, Point cam_pos) {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    valid_pos_ = false;
    if (xpos >= 0 && xpos < SCREEN_WIDTH && ypos >= 0 && ypos < SCREEN_HEIGHT) {
        int x = ((int)xpos + MESH_SIZE*cam_pos.x - (SCREEN_WIDTH - MESH_SIZE) / 2) / MESH_SIZE;
        int y = ((int)ypos + MESH_SIZE*cam_pos.y - (SCREEN_HEIGHT - MESH_SIZE) / 2) / MESH_SIZE;
        pos_ = Point{x, y};
        valid_pos_ = room_map->valid(pos_);
    }
}

void Editor::create_obj(DeltaFrame* delta_frame, RoomMap* room_map) {
    if (room_map->view(pos_, Layer::Solid)) {
        return;
    }
    int x = pos_.x;
    int y = pos_.y;
    switch (type_) {
    case 1: room_map->put(std::move(std::make_unique<Wall>(x,y)), delta_frame);
        break;
    case 2: room_map->put(std::move(std::make_unique<PushBlock>(x,y,false,StickyLevel::None)), delta_frame);
        break;
    case 3: room_map->put(std::move(std::make_unique<PushBlock>(x,y,false,StickyLevel::Weak)), delta_frame);
        break;
    case 4: room_map->put(std::move(std::make_unique<PushBlock>(x,y,false,StickyLevel::Strong)), delta_frame);
        break;
    case 5: room_map->put(std::move(std::make_unique<PushBlock>(x,y,true,StickyLevel::None)), delta_frame);
        break;
    case 6: room_map->put(std::move(std::make_unique<PushBlock>(x,y,true,StickyLevel::Weak)), delta_frame);
        break;
    case 7: room_map->put(std::move(std::make_unique<PushBlock>(x,y,true,StickyLevel::Strong)), delta_frame);
        break;
    case 8: room_map->put(std::move(std::make_unique<SnakeBlock>(x,y,false,2)), delta_frame);
        break;
    case 9: room_map->put(std::move(std::make_unique<SnakeBlock>(x,y,false,1)), delta_frame);
        break;
    default:
        break;
    }
}

void Editor::destroy_obj(DeltaFrame* delta_frame, RoomMap* room_map) {
    if (!room_map->view(pos_, Layer::Solid)) {
        return;
    }
    room_map->take(pos_, Layer::Solid, delta_frame);
}
*/
