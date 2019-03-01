#include "saveloadtab.h"

#include <iostream>

#include "room.h"
#include "roommap.h"
#include "gameobject.h"
#include "editorstate.h"
#include "player.h"

SaveLoadTab::SaveLoadTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

SaveLoadTab::~SaveLoadTab() {}

void SaveLoadTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Save/Load Tab");

    static char map_name_input[64] = "";
    ImGui::InputText(".map##SAVELOAD", map_name_input, IM_ARRAYSIZE(map_name_input));

    if (ImGui::Button("Load Map##SAVELOAD")) {
        if (!editor_->load_room(map_name_input)) {
            std::cout << "Failed to load room." << std::endl;
        }
    }

    ImGui::Separator();

    static int width = 17;
    static int height = 13;
    static int depth = 16;
    ImGui::InputInt("Room Width##SAVELOAD", &width);
    ImGui::InputInt("Room Height##SAVELOAD", &height);
    ImGui::InputInt("Room Depth##SAVELOAD", &depth);

    clamp(&width, 1, MAX_ROOM_DIMS);
    clamp(&height, 1, MAX_ROOM_DIMS);
    clamp(&depth, 1, MAX_ROOM_DIMS);

    if (ImGui::Button("Create New Map##SAVELOAD")) {
        editor_->new_room(map_name_input, width, height);
    }

    ImGui::Separator();

    static int current = 0;
    const char* room_names[1024];
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

    ImGui::Separator();

    if (ImGui::Button("Save All Maps##SAVELOAD")) {
        editor_->commit_all();
    }

    if (ImGui::Button("Begin Test Session##SAVELOAD")) {
        editor_->begin_test();
    }
}

void SaveLoadTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->map();
    if (!room_map->view(pos)) {
        auto player = room_map->view(eroom->start_pos);
        room_map->take(player);
        player->pos_ = pos;
        eroom->start_pos = pos;
        room_map->put(player);
    }
}
