#include "editorstate.h"

#include <unistd.h>


#include "gameobjectarray.h"
#include "graphicsmanager.h"
#include "gamestate.h"
#include "playingstate.h"
#include "gameobject.h"
#include "player.h"
#include "mapfile.h"

#include "saveloadtab.h"
#include "objecttab.h"
#include "doortab.h"
#include "switchtab.h"
#include "modifiertab.h"

#include "room.h"
#include "roommap.h"
#include "camera.h"

#define INIT_TAB(NAME)\
tabs_[#NAME] = std::make_unique<NAME ## Tab>(this, gfx);

EditorRoom::EditorRoom(std::unique_ptr<Room> arg_room, Point3 pos):
room {std::move(arg_room)},
start_pos {pos}, cam_pos {pos},
changed {true} {}

EditorState::EditorState(GraphicsManager* gfx): EditorBaseState(),
rooms_ {}, active_room_ {},
tabs_ {}, active_tab_ {},
objs_ {std::make_unique<GameObjectArray>()} {
    INIT_TAB(SaveLoad);
    INIT_TAB(Object);
    INIT_TAB(Door);
    INIT_TAB(Switch);
    INIT_TAB(Modifier);
    active_tab_ = tabs_["SaveLoad"].get();
}

#undef INIT_TAB

EditorState::~EditorState() {}


void EditorState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("Editor Window##ROOT", &p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (active_room_) {
        ImGui::Text(("Current Room: " + active_room_->room->name()).c_str());
        ImGui::Text("Current Height: %d", active_room_->cam_pos.z);
        if (one_layer_) {
            ImGui::Text("Only Showing This Layer (F to toggle)");
        } else {
            ImGui::Text("Showing Neighboring Layers (F to toggle)");
        }
        active_room_->changed = true;
        handle_mouse_input(active_room_->cam_pos, active_room_->room.get());
        handle_keyboard_input(active_room_->cam_pos, active_room_->room.get());
        active_room_->room->draw(gfx_, active_room_->cam_pos, ortho_cam_, one_layer_);
    }

    // Draw the editor tabs
    for (const auto& p : tabs_) {
        if (ImGui::Button((p.first + "##ROOT").c_str())) {
            active_tab_ = p.second.get();
            active_tab_->init();
        } ImGui::SameLine();
    }
    ImGui::Text(""); //This consumes the stray SameLine from the loop

    // Draw the rest of the editor GUI
    ImGui::BeginChild("Active Tab Pane##ROOT", ImVec2(500, 700), true);
    active_tab_->main_loop(active_room_);
    ImGui::EndChildFrame();
    ImGui::End();
}

void EditorState::set_active_room(std::string name) {
    active_room_ = rooms_[name].get();
}

int EditorState::get_room_names(const char* room_names[]) {
    int i = 0;
    for (auto& p : rooms_) {
        room_names[i] = p.first.c_str();
        ++i;
    }
    return i;
}

EditorRoom* EditorState::get_room(std::string name) {
    if (rooms_.count(name)) {
        return rooms_[name].get();
    }
    return nullptr;
}

void EditorState::new_room(std::string name, int w, int h) {
    if (!name.size()) {
        std::cout << "Room name must be non-empty!" << std::endl;
        return;
    }
    if (rooms_.count(name)) {
        std::cout << "A room with that name is already loaded!" << std::endl;
        return;
    }
    auto room = std::make_unique<Room>(name);
    room->initialize(*objs_, w, h, 16);
    room->room_map()->create(std::make_unique<Player>(Point3 {0,0,2}, RidingState::Free));
    room->set_cam_pos({0,0,2});
    rooms_[name] = std::make_unique<EditorRoom>(std::move(room), Point3 {0,0,2});
    set_active_room(name);
}

bool EditorState::load_room(std::string name) {
    std::string path = MAPS_MAIN + name + ".map";
    if (access(path.c_str(), F_OK) == -1) {
        return false;
    }
    MapFileI file {path};
    Point3 start_pos {0,0,2};
    std::unique_ptr<Room> room = std::make_unique<Room>(name);
    room->load_from_file(*objs_, file, &start_pos);

    //TODO: (consider?) load .mapd file here!!

    room->room_map()->create(std::make_unique<Player>(start_pos, RidingState::Free));
    room->room_map()->set_initial_state(true);
    room->set_cam_pos(start_pos);
    rooms_[name] = std::make_unique<EditorRoom>(std::move(room), start_pos);
    set_active_room(name);
    return true;
}

void EditorState::save_room(EditorRoom* eroom, bool commit) {
    std::string path;
    if (commit) {
        path = MAPS_MAIN + eroom->room->name() + ".map";
    } else {
        path = MAPS_TEMP + eroom->room->name() + ".map";
    }
    MapFileO file{path};
    eroom->room->write_to_file(file, eroom->start_pos);
}

void EditorState::unload_current_room() {
    if (!active_room_) {
        return;
    }
    rooms_.erase(active_room_->room->name());
    active_room_ = nullptr;
}

void EditorState::commit_current_room() {
    if (!active_room_) {
        return;
    }
    save_room(active_room_, true);
}

void EditorState::commit_all() {
    for (auto& p : rooms_) {
        save_room(p.second.get(), true);
    }
}

void EditorState::begin_test() {
    if (!active_room_) {
        return;
    }
    for (auto& p : rooms_) {
        if (p.second->changed) {
            p.second->changed = false;
            save_room(p.second.get(), false);
        }
    }
    auto playing_state = std::make_unique<PlayingState>(active_room_->room->name(), active_room_->start_pos, true);
    create_child(std::move(playing_state));
}

void EditorState::handle_left_click(Point3 pos) {
    active_tab_->handle_left_click(active_room_, pos);
}

void EditorState::handle_right_click(Point3 pos) {
    active_tab_->handle_right_click(active_room_, pos);
}
