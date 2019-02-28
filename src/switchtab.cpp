#include "switchtab.h"

#include "gameobject.h"
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

    // TODO: include list of all Signalers in map!!

    ImGui::Separator();

    ImGui::Text("Switches");
    ImGui::BeginChild("Switches##SWITCH", ImVec2(400, 200), true);
    for (Switch* s : switches_) {
        ObjCode obj_code = s->parent_->obj_code();
        Point3 pos = s->pos();
        ImGui::Text("%d at (%d,%d,%d)", (int)obj_code, pos.x, pos.y, pos.z);
    }
    ImGui::EndChild();

    ImGui::Text("Switchables");
    ImGui::BeginChild("Switchables##SWITCH", ImVec2(400, 200), true);
    for (Switchable* s : switchables_) {
        ObjCode obj_code = s->parent_->obj_code();
        Point3 pos = s->pos();
        ImGui::Text("%d at (%d,%d,%d)", (int)obj_code, pos.x, pos.y, pos.z);
    }
    ImGui::EndChild();

    if (ImGui::Button("Empty Queued Objects##SWITCH")) {
        switches_ = {};
        switchables_ = {};
    }

    if (!(switchables_.size() && switches_.size())) {
        return;
    }

    if (ImGui::Button("Make Signaler##SWITCH")) {
        auto signaler = std::make_unique<Signaler>(0, switches_.size(), false, false);
        for (auto& obj : switches_) {
            signaler->push_switch_mutual(obj);
        }
        for (auto& obj : switchables_) {
            signaler->push_switchable_mutual(obj);
        }
        eroom->room->room_map()->push_signaler(std::move(signaler));
        switches_ = {};
        switchables_ = {};
    }
}

void SwitchTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    if (GameObject* obj = eroom->room->room_map()->view(pos)) {
        if (ObjectModifier* mod = obj->modifier()) {
            auto swble = dynamic_cast<Switchable*>(mod);
            if (swble && std::find(switchables_.begin(), switchables_.end(), swble) == switchables_.end()) {
                switchables_.push_back(swble);
                return;
            }
            auto sw = dynamic_cast<Switch*>(mod);
            if (sw && std::find(switches_.begin(), switches_.end(), sw) == switches_.end())  {
                switches_.push_back(sw);
                return;
            }
        }
    }
}

void SwitchTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    if (GameObject* obj = eroom->room->room_map()->view(pos)) {
        ObjectModifier* mod = obj->modifier();
        switchables_.erase(std::remove(switchables_.begin(), switchables_.end(), mod), switchables_.end());
        switches_.erase(std::remove(switches_.begin(), switches_.end(), mod), switches_.end());
    }
}
