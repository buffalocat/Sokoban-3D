#include "editorstate.h"

#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <dear/imgui.h>
#pragma GCC diagnostic pop

#include "graphicsmanager.h"
#include "gamestate.h"
#include "playingstate.h"

#include "room.h"
#include "roommap.h"
#include "camera.h"
#include "block.h"
#include "switch.h"
#include "door.h"


EditorRoom::EditorRoom(std::unique_ptr<Room> arg_room, Point pos):
room {std::move(arg_room)},
start_pos {pos}, cam_pos {pos},
changed {true} {}

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

void EditorTab::handle_left_click(EditorRoom* eroom, Point pos) {}

void EditorTab::handle_right_click(EditorRoom* eroom, Point pos) {}

SaveLoadTab::SaveLoadTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

void SaveLoadTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Save/Load Tab");

    static char map_name_input[64] = "";
    ImGui::InputText(".map##SAVELOAD", map_name_input, IM_ARRAYSIZE(map_name_input));

    if (ImGui::Button("Load Map##SAVELOAD")) {
        if (!editor_->load_room(map_name_input)) {
            std::cout << "Failed to load room." << std::endl;
        }
    }

    static int width = 17;
    static int height = 13;
    ImGui::InputInt("Room Width##SAVELOAD", &width);
    ImGui::InputInt("Room Height##SAVELOAD", &height);
    clamp(&width, 1, MAX_ROOM_DIMS);
    clamp(&height, 1, MAX_ROOM_DIMS);

    if (ImGui::Button("Create New Map##SAVELOAD")) {
        editor_->new_room(map_name_input, width, height);
    }

    static int current = 0;
    const char* room_names[256];
    int len = editor_->get_room_names(room_names);
    if (ImGui::ListBox("Loaded Maps##SAVELOAD", &current, room_names, len, len)) {
        editor_->set_active_room(std::string(room_names[current]));
    }

    if (ImGui::Button("Unload Current Map##SAVELOAD")) {
        editor_->unload_current_room();
    }

    if (ImGui::Button("Save Current Map##SAVELOAD")) {
        editor_->commit_current_room();
    }

    if (ImGui::Button("Save All Maps##SAVELOAD")) {
        editor_->commit_all();
    }

    if (ImGui::Button("Begin Test Session##SAVELOAD")) {
        editor_->begin_test();
    }
}

void SaveLoadTab::handle_left_click(EditorRoom* eroom, Point pos) {
    RoomMap* room_map = eroom->room->room_map();
    if (!room_map->view(pos, Layer::Player)) {
        Player* player = static_cast<Player*>(room_map->view(eroom->start_pos, ObjCode::Player));
        auto player_unique = room_map->take_quiet(player);
        player->set_pos(pos);
        eroom->start_pos = pos;
        room_map->put_quiet(std::move(player_unique));
    }
}


ObjectTab::ObjectTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
layer {(int)Layer::Solid}, obj_code {(int)ObjCode::NONE},
color {GREEN}, pb_sticky {(int)StickyLevel::None},
is_car {true}, sb_ends {2} {}

void ObjectTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Object Tab");
}


#define INIT_TAB(NAME)\
tabs_[#NAME] = std::make_unique<NAME ## Tab>(this, gfx);

EditorState::EditorState(GraphicsManager* gfx): GameState(gfx),
rooms_ {}, active_room_ {},
tabs_ {}, active_tab_ {},
ortho_cam_ {true}, keyboard_cooldown_ {0} {
    INIT_TAB(SaveLoad);
    INIT_TAB(Object);
    active_tab_ = tabs_["SaveLoad"].get();
}

#undef INIT_TAB

Point EditorState::get_pos_from_mouse() {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    Point cam_pos = active_room_->cam_pos;
    if (xpos >= 0 && xpos < SCREEN_WIDTH && ypos >= 0 && ypos < SCREEN_HEIGHT) {
        int x = ((int)xpos + MESH_SIZE*cam_pos.x - (SCREEN_WIDTH - MESH_SIZE) / 2) / MESH_SIZE;
        int y = ((int)ypos + MESH_SIZE*cam_pos.y - (SCREEN_HEIGHT - MESH_SIZE) / 2) / MESH_SIZE;
        return Point{x, y};
    }
    return Point{-1, -1};
}

bool EditorState::want_capture_keyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}

bool EditorState::want_capture_mouse() {
    return ImGui::GetIO().WantCaptureMouse;
}

void EditorState::handle_mouse_input() {
    if (want_capture_mouse() || !ortho_cam_) {
        return;
    }
    Point mouse_pos = get_pos_from_mouse();
    if (!active_room_->room->valid(mouse_pos)) {
        return;
    }
    if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        active_tab_->handle_left_click(active_room_, mouse_pos);
    } else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        active_tab_->handle_right_click(active_room_, mouse_pos);
    }
}

void EditorState::clamp_to_active_map(Point& pos) {
    RoomMap* cur_map = active_room_->room->room_map();
    pos = Point {
        std::max(0, std::min(cur_map->width() - 1, pos.x)),
        std::max(0, std::min(cur_map->height() - 1, pos.y))
    };
}

void EditorState::handle_keyboard_input() {
    if (keyboard_cooldown_ > 0) {
        --keyboard_cooldown_;
        return;
    }
    if (want_capture_keyboard()) {
        return;
    }
    keyboard_cooldown_ = MAX_COOLDOWN;
    for (auto p : MOVEMENT_KEYS) {
        if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
            if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                active_room_->cam_pos += FAST_MAP_MOVE * p.second;
            } else {
                active_room_->cam_pos += p.second;
            }
            clamp_to_active_map(active_room_->cam_pos);
            return;
        }
    }
    if (glfwGetKey(window_, GLFW_KEY_C)) {
        ortho_cam_ = !ortho_cam_;
        return;
    }
    keyboard_cooldown_ = 0;
}

void EditorState::main_loop() {
    bool p_open = true;
    if (!ImGui::Begin("My Editor Window##ROOT", &p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    if (active_room_) {
        handle_mouse_input();
        handle_keyboard_input();
        active_room_->room->draw(gfx_, active_room_->cam_pos, ortho_cam_);
    }

    // Draw the editor tabs
    for (const auto& p : tabs_) {
        if (ImGui::Button((p.first + "##ROOT").c_str())) {
            active_tab_ = p.second.get();
        } ImGui::SameLine();
    }
    ImGui::Text(""); //This consumes the stray SameLine from the loop

    // Draw the rest of the editor GUI
    ImGui::BeginChild("Active Tab Pane##ROOT", ImVec2(400, 600), true);
    active_tab_->main_loop(active_room_);
    ImGui::EndChildFrame();
    ImGui::End();
}

void EditorState::set_active_room(std::string name) {
    if (rooms_.count(name)) {
        active_room_ = rooms_[name].get();
    } else {
        std::cout << "Tried to switch active room to nonexistent room. Weird!" << std::endl;
    }
}

int EditorState::get_room_names(const char* room_names[]) {
    int i = 0;
    for (auto& p : rooms_) {
        room_names[i] = p.first.c_str();
        ++i;
    }
    return i;
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
    auto room = std::make_unique<Room>(name, w, h);
    room->room_map()->put_quiet(std::make_unique<Player>(0, 0, RidingState::Free));
    room->set_cam_pos(Point {0,0});
    rooms_[name] = std::make_unique<EditorRoom>(std::move(room), Point {0,0});
    set_active_room(name);
}

bool EditorState::load_room(std::string name) {
    std::string path = MAPS_MAIN + name + ".map";
    if (access(path.c_str(), F_OK) == -1) {
        return false;
    }
    std::ifstream file;
    file.open(path, std::ios::in | std::ios::binary);
    Point start_pos {0,0};
    std::unique_ptr<Room> room = std::make_unique<Room>(name);
    room->load_from_file(file, &start_pos);
    file.close();

    //NOTE: Later, load .mapd file here!!

    room->room_map()->put_quiet(std::make_unique<Player>(start_pos.x, start_pos.y, RidingState::Free));
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
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);
    eroom->room->write_to_file(file, eroom->start_pos);
    file.close();
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
    auto playing = std::make_unique<PlayingState>(gfx_, active_room_->room->name(), active_room_->start_pos, true);
    create_child(std::move(playing));
}

