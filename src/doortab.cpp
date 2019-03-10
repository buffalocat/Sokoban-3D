#include "doortab.h"

#include "room.h"
#include "roommap.h"
#include "gameobject.h"
#include "door.h"
#include "editorstate.h"
#include "doorselectstate.h"

DoorTab::DoorTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
entrance_ {}, exit_room_ {}, exit_pos_ {-1,-1,-1} {}

DoorTab::~DoorTab() {}

void DoorTab::init() {
    entrance_ = nullptr;
    exit_room_ = nullptr;
    exit_pos_ = {-1,-1,-1};
}

static bool failed_to_load_room = false;

void DoorTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Door Tab");
    ImGui::Separator();
    if (!eroom) {
        ImGui::Text("No room loaded.");
        return;
    }

    ImGui::Text("Click on a door to select it.");
    if (!entrance_) {
        return;
    }

    ImGui::Separator();

    Point3 pos = entrance_->pos();
    ImGui::Text("Currently selected door: (%d,%d,%d)", pos.x, pos.y, pos.z);
    MapLocation* dest = entrance_->dest();
    if (dest) {
        ImGui::Text("This door has destination \"%s\":(%d,%d,%d) [offset pos]", dest->name.c_str(), dest->pos.x, dest->pos.y, dest->pos.z);
        if (!exit_room_) {
            ImGui::Text("...but that room's .map file doesn't appear to exist.");
        }
    } else {
        ImGui::Text("This door doesn't have a destination yet");
    }

    static int current = 0;
    const char* room_names[256];
    int len = editor_->get_room_names(room_names);
    if (ImGui::ListBox("Loaded Maps##DOOR", &current, room_names, len, len)) {
        if (exit_room_ != editor_->get_room(room_names[current])) {
            exit_room_ = editor_->get_room(room_names[current]);
            exit_pos_ = {-1,-1,-1};
        }
    }

    if (!exit_room_) {
        ImGui::Text("No room selected");
        return;
    }

    ImGui::Text(("Destination room: " + exit_room_->name()).c_str());

    if (ImGui::Button("Select Destination Position##DOOR")) {
        auto select_state = std::make_unique<DoorSelectState>(exit_room_->room.get(), exit_room_->start_pos, &exit_pos_);
        editor_->create_child(std::move(select_state));
    }

    if (exit_pos_.x == -1) {
        ImGui::Text("No destination selected");
        return;
    }

    ImGui::Text("Destination Position: (%d,%d,%d)", exit_pos_.x, exit_pos_.y, exit_pos_.z);
    if (GameObject* obj = eroom->map()->view(exit_pos_)) {
        ImGui::Text(obj->to_str().c_str());
        if (Door* exit_door = dynamic_cast<Door*>(obj->modifier())) {
            ImGui::Text("The exit position is also a door.");
            if (ImGui::Button("Link Both Ways##DOOR")) {
                entrance_->set_dest(Point3_S16{exit_pos_ - Point3{exit_room_->room->offset_pos_}}, exit_room_->name());
                exit_door->set_dest(Point3_S16{entrance_->pos() - Point3{eroom->room->offset_pos_}}, eroom->name());
                // We changed something remotely!
                exit_room_->changed = true;
            }
        }
    } else {
        ImGui::Text("Empty");
    }

    if (ImGui::Button("Link One Way##DOOR")) {
        entrance_->set_dest(Point3_S16{exit_pos_ - Point3{exit_room_->room->offset_pos_}}, exit_room_->name());
    }
}

void DoorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    if (GameObject* obj = eroom->map()->view(pos)) {
        if (Door* door = dynamic_cast<Door*>(obj->modifier())) {
            entrance_ = door;
            MapLocation* dest = door->dest();
            if (dest) {
                // Try to load the destination room
                exit_room_ = editor_->get_room(dest->name);
                if (!exit_room_) {
                    editor_->load_room(dest->name, true);
                    exit_room_ = editor_->get_room(dest->name);
                }
                if (exit_room_) {
                    exit_pos_ = Point3{dest->pos} + Point3{exit_room_->room->offset_pos_};
                } else {
                    exit_pos_ = {-1,-1,-1};
                }
            }
        }
    }
}
