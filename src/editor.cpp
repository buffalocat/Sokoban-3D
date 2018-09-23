#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include <dear/imgui.h>

#pragma GCC diagnostic pop

#include "editor.h"
#include "room.h"
#include "block.h"

Editor::Editor(GLFWwindow* window): window_ {window}, room_ {}, pos_ {Point{0,0}},
mode_ {EditorMode::SaveLoad},
solid_obj {(int)ObjCode::NONE}, pb_sticky {(int)StickyLevel::None},
is_car {true}, sb_ends {2}
{}

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

void Editor::set_room(Room* room) {
    room_ = room;
}

void Editor::ShowMainWindow(bool* p_open) {
    if (!ImGui::Begin("My Editor Window", p_open, 0)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Save/Load")) {
        mode_ = EditorMode::SaveLoad;
    } ImGui::SameLine();
    if (ImGui::Button("Create/Delete Objects")) {
        mode_ = EditorMode::Objects;
    } ImGui::SameLine();
    if (ImGui::Button("Camera")) {
        mode_ = EditorMode::Camera;
    }

    switch (mode_) {
    case EditorMode::SaveLoad :
        draw_saveload_tab();
        break;
    case EditorMode::Objects :
        draw_objects_tab();
        break;
    case EditorMode::Camera :
        draw_camera_tab();
        break;
    default:
        break;
    }

    ImGui::End();
}

void Editor::draw_saveload_tab() {
    static char buf[32] = "";
    ImGui::InputText(".map", buf, IM_ARRAYSIZE(buf));
    if (ImGui::Button("Load Map")) {
        room_->load(buf);
    }
    if (ImGui::Button("Save Map")) {
        room_->save(buf, false);
    }
    if (ImGui::Button("Save Map (Force Overwrite)")) {
        room_->save(buf, true);
    }
}

void Editor::draw_objects_tab() {
    ImGui::RadioButton("Wall", &solid_obj, (int)ObjCode::Wall);
    ImGui::RadioButton("PushBlock", &solid_obj, (int)ObjCode::PushBlock);
    ImGui::RadioButton("SnakeBlock", &solid_obj, (int)ObjCode::SnakeBlock);

    ImGui::Text("Object Properties");

    if (solid_obj == (int)ObjCode::Wall) { }
    else if (solid_obj == (int)ObjCode::PushBlock) {
        ImGui::Text("Stickiness");
        ImGui::RadioButton("Not Sticky", &pb_sticky, (int)StickyLevel::None);
        ImGui::RadioButton("Weakly Sticky", &pb_sticky, (int)StickyLevel::Weak);
        ImGui::RadioButton("Strongly Sticky", &pb_sticky, (int)StickyLevel::Strong);

        ImGui::Checkbox("Is Player?", &is_car);
    }
    else if (solid_obj == (int)ObjCode::SnakeBlock) {
        ImGui::Text("Number of Ends");
        ImGui::RadioButton("One Ended", &sb_ends, 1);
        ImGui::RadioButton("Two Ended", &sb_ends, 2);

        ImGui::Checkbox("Is Player?", &is_car);
    }
}

void Editor::draw_camera_tab() {

}

void Editor::handle_input() {
    if (ImGui::IsMouseHoveringAnyWindow()) {
        return;
    }
    Point mouse_pos = room_->get_pos_from_mouse();
    if (!room_->valid(mouse_pos)) {
        return;
    }
    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        room_->create_obj(mouse_pos);
    } else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        room_->delete_obj(mouse_pos);
    }
}

std::unique_ptr<GameObject> Editor::create_obj(Point pos) {
    int x = pos.x;
    int y = pos.y;
    switch (solid_obj) {
    case (int)ObjCode::Wall :
        return std::make_unique<Wall>(x, y);
    case (int)ObjCode::PushBlock :
        return std::make_unique<PushBlock>(x, y, is_car, static_cast<StickyLevel>(pb_sticky));
    case (int)ObjCode::SnakeBlock :
        return std::make_unique<SnakeBlock>(x, y, is_car, sb_ends);
    }
    return nullptr;
}
