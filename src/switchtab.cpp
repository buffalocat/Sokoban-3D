#include "switchtab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "switch.h"

SwitchTab::SwitchTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
switchables_ {}, switches_ {} {}

SwitchTab::~SwitchTab() {}

void SwitchTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Switch Tab");

    if (!(switchables_.size() && switches_.size())) {
        return;
    }

    if (ImGui::Button("Make Signaler##SWITCH")) {
        auto signaler = std::make_unique<Signaler>(switches_.size(), false, false);
        for (auto& obj : switches_) {
            signaler->push_switch(obj);
        }
        for (auto& obj : switchables_) {
            signaler->push_switchable(obj);
        }
        eroom->room->push_signaler(std::move(signaler));
        switchables_ = {};
        switches_ = {};
    }
}

void SwitchTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->room->room_map();
    auto swble = dynamic_cast<Switchable*>(room_map->view(pos));
    if (swble) {
        switchables_.insert(swble);
        return;
    }
    auto sw = dynamic_cast<Switch*>(room_map->view(pos));
    if (sw) {
        switches_.insert(sw);
        return;
    }
}

void SwitchTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    switchables_ = {};
    switches_ = {};
}
