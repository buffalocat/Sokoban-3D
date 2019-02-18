#include "switchtab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"
#include "switch.h"
#include "signaler.h"
#include "switchable.h"

#include <algorithm>

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
        eroom->room->room_map()->push_signaler(std::move(signaler));
        switchables_ = {};
        switches_ = {};
    }
}

void SwitchTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    /*
    RoomMap* room_map = eroom->room->room_map();
    auto swble = dynamic_cast<Switchable*>(room_map->view(pos));
    if (swble && std::find(switchables_.begin(), switchables_.end(), swble) == switchables_.end()) {
        switchables_.push_back(swble);
        return;
    }
    auto sw = dynamic_cast<Switch*>(room_map->view(pos));
    if (sw && std::find(switches_.begin(), switches_.end(), sw) == switches_.end())  {
        switches_.push_back(sw);
        return;
    }
    */
}

void SwitchTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    /*
    GameObject* obj = eroom->room->room_map()->view(pos);
    switchables_.erase(std::remove(switchables_.begin(), switchables_.end(), obj), switchables_.end());
    switches_.erase(std::remove(switches_.begin(), switches_.end(), obj), switches_.end());
    */
}
