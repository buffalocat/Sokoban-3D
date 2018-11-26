#include "doortab.h"

#include "room.h"
#include "roommap.h"
#include "door.h"
#include "editorstate.h"
#include "doorselectstate.h"

DoorTab::DoorTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
entrance_ {}, exit_name_ {""}, exit_pos_ {-1, -1, -1}, exit_ {} {}

DoorTab::~DoorTab() {}

void DoorTab::init() {
    entrance_ = nullptr;
    exit_name_ = "";
    exit_pos_ = {-1, -1, -1};
    exit_ = nullptr;
}

void DoorTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Door Tab");

    ImGui::Text("Click on a door to select it.");
    if (!entrance_) {
        entrance_ = nullptr;
        return;
    }

    Point3 pos = entrance_->pos();
    ImGui::Text("Currently selected door: (%d,%d,%d)", pos.x, pos.y, pos.z);
    MapLocation* dest = entrance_->dest();
    if (dest) {
        ImGui::Text("This door has destination \"%s\":(%d,%d,%d)", dest->name.c_str(), dest->pos.x, dest->pos.y, dest->pos.z);
    } else {
        ImGui::Text("This door doesn't have a destination yet");
    }

    static int current = 0;
    const char* room_names[256];
    int len = editor_->get_room_names(room_names);
    if (ImGui::ListBox("Loaded Maps##DOOR", &current, room_names, len, len)) {
        if (exit_name_.compare(room_names[current])) {
            exit_name_ = std::string(room_names[current]);
            exit_pos_ = {-1, -1, -1};
        }
    }

    if (exit_name_.size() == 0) {
        ImGui::Text("No room selected");
        return;
    }

    ImGui::Text(("Destination room: " + exit_name_).c_str());

    if (ImGui::Button("Select Destination Position##DOOR")) {
        EditorRoom* dest_room = editor_->get_room(exit_name_);
        auto select_state = std::make_unique<DoorSelectState>(dest_room->room.get(), dest_room->start_pos, &exit_pos_, &exit_);
        editor_->create_child(std::move(select_state));
    }

    if (exit_pos_.x == -1) {
        ImGui::Text("No destination selected");
        return;
    }

    ImGui::Text("Destination Position: (%d,%d,%d)", exit_pos_.x, exit_pos_.y, exit_pos_.z);

    if (exit_) {
        ImGui::Text("The exit position is also a door.");
        if (ImGui::Button("Link Both Ways#DOOR")) {
            entrance_->set_dest(exit_pos_, exit_name_);
            exit_->set_dest(entrance_->pos(), eroom->room->name());
            // We changed something remotely!
            editor_->get_room(exit_name_)->changed = true;
        }
    }
    if (ImGui::Button("Link One Way#DOOR")) {
        entrance_->set_dest(exit_pos_, exit_name_);
    }
}

void DoorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    Door* door = dynamic_cast<Door*>(eroom->room->room_map()->view(pos));
    if (door) {
        entrance_ = door;
    }
}
