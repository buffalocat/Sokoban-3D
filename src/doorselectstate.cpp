#include "doorselectstate.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "door.h"

// TODO: make sure the validity checks make sense

DoorSelectState::DoorSelectState(Room* room, Point3 cam_pos, Point3* exit_pos):
EditorBaseState(), room_ {room}, cam_pos_ {cam_pos}, exit_pos_ {exit_pos} {}

DoorSelectState::~DoorSelectState() {}

void DoorSelectState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("Door Destination Select Window##DOOR", &p_open)) {
        ImGui::End();
        return;
    }

    handle_mouse_input(cam_pos_, room_);
    handle_keyboard_input(cam_pos_, room_);
    room_->draw(gfx_, cam_pos_, true, one_layer_);

    display_hover_pos_object(cam_pos_, room_->map());

    ImGui::Separator();

    if (exit_pos_->x == -1) {
        ImGui::Text("Destination not selected.");
    } else {
        ImGui::Text("Destination Pos: (%d,%d,%d)", exit_pos_->x, exit_pos_->y, exit_pos_->z);
        if (GameObject* obj = room_->map()->view(*exit_pos_)) {
            ImGui::Text(obj->to_str().c_str());
        } else {
            ImGui::Text("Empty");
        }
    }
    ImGui::Text("Press escape to return.");

    ImGui::End();
}

void DoorSelectState::handle_left_click(Point3 pos) {
    if (pos.x == -1) {
        return;
    }
    *exit_pos_ = pos;
}

void DoorSelectState::handle_right_click(Point3 pos) {}
