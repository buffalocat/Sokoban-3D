#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include <dear/imgui.h>

#pragma GCC diagnostic pop

#include <map>

#include "editor.h"
#include "roommanager.h"
#include "block.h"
#include "door.h"

Editor::Editor(GLFWwindow* window, RoomManager* mgr): window_ {window}, mgr_ {mgr}, pos_ {Point{0,0}},
save_load_tab_ {SaveLoadTab(mgr)},
object_tab_ {ObjectTab(mgr)},
camera_tab_ {CameraTab(mgr)},
door_tab_ {DoorTab(mgr)}
{
    mgr->set_editor(this);
    active_tab_ = &save_load_tab_;
}

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

void Editor::ShowMainWindow(bool* p_open) {
    if (!ImGui::Begin("My Editor Window", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Save/Load##TAB_SELECT")) {
        active_tab_ = &save_load_tab_;
    } ImGui::SameLine();
    if (ImGui::Button("Create/Delete Objects##TAB_SELECT")) {
        active_tab_ = &object_tab_;
    } ImGui::SameLine();
    if (ImGui::Button("Camera##TAB_SELECT")) {
        active_tab_ = &camera_tab_;
    } ImGui::SameLine();
    if (ImGui::Button("Door##TAB_SELECT")) {
        active_tab_ = &door_tab_;
    }

    ImGui::BeginChild("active tab pane", ImVec2(400, 600), true);
    active_tab_->draw();
    ImGui::EndChildFrame();

    ImGui::End();
}

bool Editor::want_capture_keyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool Editor::want_capture_mouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

void Editor::handle_input() {
    if (want_capture_mouse()) {
        return;
    }
    Point mouse_pos = mgr_->get_pos_from_mouse();
    if (!mgr_->valid(mouse_pos)) {
        return;
    }
    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        active_tab_->handle_left_click(mouse_pos);
    } else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        active_tab_->handle_right_click(mouse_pos);
    }
}

void SaveLoadTab::draw() {
    static char map_name[64] = "";
    static std::map<std::string, bool> loaded_rooms;
    ImGui::InputText(".map", map_name, IM_ARRAYSIZE(map_name));
    if (ImGui::Button("Load Map")) {
        mgr_->init_load(map_name);
    }
    if (ImGui::Button("Save Map")) {
        mgr_->save(map_name, false, true);
    }
    if (ImGui::Button("Save Map (Force Overwrite)")) {
        mgr_->save(map_name, true, true);
    }
}

void ObjectTab::draw() {
    ImGui::RadioButton("Wall##OBJECT_SELECT", &obj_code, (int)ObjCode::Wall);
    ImGui::RadioButton("PushBlock##OBJECT_SELECT", &obj_code, (int)ObjCode::PushBlock);
    ImGui::RadioButton("SnakeBlock##OBJECT_SELECT", &obj_code, (int)ObjCode::SnakeBlock);
    ImGui::RadioButton("Door##OBJECT_SELECT", &obj_code, (int)ObjCode::Door);

    ImGui::Text("Object Properties");

    if (obj_code == (int)ObjCode::Wall) {}
    else if (obj_code == (int)ObjCode::PushBlock) {
        ImGui::Text("Stickiness");
        ImGui::RadioButton("Not Sticky", &pb_sticky, (int)StickyLevel::None);
        ImGui::RadioButton("Weakly Sticky", &pb_sticky, (int)StickyLevel::Weak);
        ImGui::RadioButton("Strongly Sticky", &pb_sticky, (int)StickyLevel::Strong);

        ImGui::Checkbox("Is Player?", &is_car);
    }
    else if (obj_code == (int)ObjCode::SnakeBlock) {
        ImGui::Text("Number of Ends");
        ImGui::RadioButton("One Ended", &sb_ends, 1);
        ImGui::RadioButton("Two Ended", &sb_ends, 2);

        ImGui::Checkbox("Is Player?", &is_car);
    }
    else if (obj_code == (int)ObjCode::Door) {}

}

void CameraTab::draw() {
    ImGui::Text("Camera Type");

    ImGui::Text("Corner Positions");
    ImGui::InputInt("x1##CAM CORNER", &x1);
    ImGui::InputInt("y1##CAM CORNER", &y1);
    ImGui::InputInt("x2##CAM CORNER", &x2);
    ImGui::InputInt("y2##CAM CORNER", &y2);

    if (ImGui::Button("Create Camera Rect")) {
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        int w = abs(x1 - x2) + 1;
        int h = abs(y1 - y2) + 1;
        camera_->push_context(std::make_unique<FixedCameraContext>(x, y, w, h, priority, radius, x + w/2.0, y + h/2.0));
    }
}

void DoorTab::draw() {
    ImGui::Text("Destination Room");
    static char buf[64] = "";
    ImGui::InputText(".map", buf, IM_ARRAYSIZE(buf));

    static int x1 = 0;
    static int y1 = 0;
    ImGui::Text("Door Position");
    ImGui::InputInt("door x", &x1);
    ImGui::InputInt("door y", &y1);

    static int x2 = 0;
    static int y2 = 0;
    ImGui::Text("Destination Position");
    ImGui::InputInt("dest x", &x2);
    ImGui::InputInt("dest y", &y2);

    if (ImGui::Button("(try to) Make That Door!!")) {
        mgr_->make_door(Point{x1,y1}, Point{x2,y2}, std::string(buf));
    }
}

void SaveLoadTab::handle_left_click(Point) {}
void SaveLoadTab::handle_right_click(Point) {}

void ObjectTab::handle_left_click(Point pos) {
    int x = pos.x;
    int y = pos.y;
    std::unique_ptr<GameObject> obj;
    switch (obj_code) {
    case (int)ObjCode::Wall :
        obj = std::make_unique<Wall>(x, y);
        break;
    case (int)ObjCode::PushBlock :
        obj = std::make_unique<PushBlock>(x, y, is_car, static_cast<StickyLevel>(pb_sticky));
        break;
    case (int)ObjCode::SnakeBlock :
        obj = std::make_unique<SnakeBlock>(x, y, is_car, sb_ends);
        break;
    case (int)ObjCode::Door :
        obj = std::make_unique<Door>(x, y);
        break;
    default:
        return;
    }
    mgr_->create_obj(std::move(obj));
}

void ObjectTab::handle_right_click(Point pos) {
    mgr_->delete_obj(pos);
}

void CameraTab::handle_left_click(Point pos) {
    x1 = pos.x;
    y1 = pos.y;
}

void CameraTab::handle_right_click(Point pos) {
    x2 = pos.x;
    y2 = pos.y;
}

void DoorTab::handle_left_click(Point) {}
void DoorTab::handle_right_click(Point) {}

EditorTab::EditorTab(RoomManager* mgr): mgr_ {mgr} {}
EditorTab::~EditorTab() {}

SaveLoadTab::SaveLoadTab(RoomManager* mgr): EditorTab(mgr) {}
SaveLoadTab::~SaveLoadTab() {}

ObjectTab::ObjectTab(RoomManager* mgr): EditorTab(mgr),
obj_code {(int)ObjCode::NONE}, pb_sticky {(int)StickyLevel::None},
is_car {true}, sb_ends {2} {}
ObjectTab::~ObjectTab() {}

CameraTab::CameraTab(RoomManager* mgr): EditorTab(mgr), camera_ {mgr->camera()},
x1 {0}, y1 {0}, x2 {0}, y2 {0},
radius {6.0}, priority {10} {}
CameraTab::~CameraTab() {}

DoorTab::DoorTab(RoomManager* mgr): EditorTab(mgr) {}
DoorTab::~DoorTab() {}
