#include "editorstate.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <dear/imgui.h>
#pragma GCC diagnostic pop

#include "graphicsmanager.h"
#include "gamestate.h"

#include "roommanager.h"
#include "camera.h"
#include "room.h"
#include "block.h"
#include "switch.h"
#include "door.h"

/*
EditorRoomSet::EditorRoomSet(): RoomSet(), active_room_ {}, changed_ {} {}

bool EditorRoomSet::activate(std::string name) {
    if (rooms_.count(name) == 0) {
        if (!load(MAPS_DIR, name)) {
            return false;
        }
    }
    active_room_ = rooms_[name].get();
    return true;
}

void EditorRoomSet::save_all(bool commit) {
    set_changed(active_room_.name());
    std::string base = commit ? "maps\\main\\" : "maps\\edit\\";
    for (auto& name : changed_) {
        std::string path = base + name;
        Room* room = rooms_[name].get();
        save(path, room);
    }
    changed_ = {active_room_.name()};
}

void EditorRoomSet::set_changed(std::string name) {
    changed_.insert(name);
}

void EditorRoomSet::discard_current() {

}

void EditorRoomSet::discard_all() {

} //*/

EditorTab::EditorTab(EditorState* editor, GraphicsManager* gfx): editor_ {editor}, gfx_ {gfx} {}

EditorTab::~EditorTab() {}

#define INIT_TAB(NAME)\
tabs_[#NAME] = std::make_unique<NAME ## Tab>(this, gfx);

EditorState::EditorState(GraphicsManager* gfx):
rooms_ {}, active_room_ {},
tabs_ {}, active_tab_ {},
gfx_ {gfx}, window_{gfx->window()}, ortho_cam_ {true} {
    INIT_TAB(SaveLoad);
    INIT_TAB(Object);
    active_tab_ = tabs_["SaveLoad"].get();
}

#undef INIT_TAB


SaveLoadTab::SaveLoadTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

void SaveLoadTab::main_loop(EditorRoom* eroom) {

}


ObjectTab::ObjectTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
layer {(int)Layer::Solid}, obj_code {(int)ObjCode::NONE},
color {GREEN}, pb_sticky {(int)StickyLevel::None},
is_car {true}, sb_ends {2} {}

void ObjectTab::main_loop(EditorRoom* eroom) {

}

/*
constexpr std::string tab_select_label(std::string tab_name) {
    return tab_name + "##TAB_SELECT";
}  //*/

void EditorState::main_loop() {
    // Draw the current room
    if (active_room_) {
        // active_room_->room->draw(ortho_cam_, active_room_->cam_pos);
    }

    bool p_open = true;
    if (!ImGui::Begin("My Editor Window", &p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    // Draw the editor tabs
    for (const auto& p : tabs_) {
        if (ImGui::Button((p.first + "##TAB_SELECT").c_str())) {
            active_tab_ = p.second.get();
        } ImGui::SameLine();
    }
    ImGui::Text(""); //This consumes the stray SameLine from the loop

    // Draw the rest of the editor GUI
    ImGui::BeginChild("active_tab_pane", ImVec2(400, 600), true);
    //active_tab_->main_loop(active_room_);
    ImGui::EndChildFrame();
    ImGui::End();
}
