#include "doorselectstate.h"

#include "gameobject.h"
#include "room.h"
#include "roommap.h"
#include "door.h"

// TODO: make sure the validity checks make sense

DoorSelectState::DoorSelectState(Room* room, Point3 cam_pos, Point3* door_pos, Door** door, bool* valid):
EditorBaseState(), room_ {room}, cam_pos_ {cam_pos}, door_pos_ {door_pos}, door_ {door}, valid_ {valid} {}

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

    if (!*valid_) {
        ImGui::Text("Destination not selected.");
    } else {
        ImGui::Text("Destination Position: (%d,%d,%d)", door_pos_->x, door_pos_->y, door_pos_->z);
    }
    ImGui::Text("Press escape to return.");

    ImGui::End();
}

void DoorSelectState::handle_left_click(Point3 pos) {
    *door_pos_ = pos;
    *valid_ = true;
    if (GameObject* obj = room_->room_map()->view(*door_pos_)) {
        *door_ = dynamic_cast<Door*>(obj->modifier());
    }
}

void DoorSelectState::handle_right_click(Point3 pos) {}
