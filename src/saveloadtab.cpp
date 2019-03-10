#include "saveloadtab.h"

#include <iostream>

#include "common_constants.h"

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
        if (editor_->load_room(map_name_input, true)) {
            editor_->set_active_room(map_name_input);
        } else {
            std::cout << "Failed to load room." << std::endl;
        }
    }

    ImGui::Separator();

    static int width = 17;
    static int height = 13;
    static int depth = 4;
    ImGui::InputInt("Room Width##SAVELOAD", &width);
    ImGui::InputInt("Room Height##SAVELOAD", &height);
    ImGui::InputInt("Room Depth##SAVELOAD", &depth);

    clamp(&width, 1, MAX_ROOM_DIMS);
    clamp(&height, 1, MAX_ROOM_DIMS);
    clamp(&depth, 1, MAX_ROOM_DIMS);

    if (ImGui::Button("Create New Map##SAVELOAD")) {
        editor_->new_room(map_name_input, width, height, depth);
    }

    ImGui::Separator();

    static int current = 0;
    const char* room_names[1024];
    int len = editor_->get_room_names(room_names);
    if (ImGui::ListBox("Loaded Maps##SAVELOAD", &current, room_names, len, len)) {
        editor_->set_active_room(std::string(room_names[current]));
    }

    ImGui::Separator();

    if (ImGui::Button("Save All Maps##SAVELOAD")) {
        editor_->commit_all();
    }

    if (ImGui::Button("Begin Test Session##SAVELOAD")) {
        editor_->begin_test();
    }

    ImGui::Separator();

    if (!eroom) {
        return;
    }

    if (ImGui::Button("Save Current Map##SAVELOAD")) {
        editor_->commit_current_room();
    }

    Point3 cur_room_dims { eroom->map()->width_, eroom->map()->height_, eroom->map()->depth_};
    ImGui::Text("Current Room Dimensions: (%d,%d,%d)", cur_room_dims.x, cur_room_dims.y, cur_room_dims.z);

    static int extend_width;
    static int extend_height;
    static int extend_depth;

    ImGui::InputInt("Extend Width##SAVELOAD", &extend_width);
    ImGui::InputInt("Extend Height##SAVELOAD", &extend_height);
    ImGui::InputInt("Extend Depth##SAVELOAD", &extend_depth);

    clamp(&extend_width, 1 - cur_room_dims.x, MAX_ROOM_DIMS - cur_room_dims.x);
    clamp(&extend_height, 1 - cur_room_dims.y, MAX_ROOM_DIMS - cur_room_dims.y);
    clamp(&extend_depth, 1 - cur_room_dims.z, MAX_ROOM_DIMS - cur_room_dims.z);

    if (ImGui::Button("Extend room?##SAVELOAD")) {
        Point3 dpos = {extend_width, extend_height, extend_depth};
        eroom->room->extend_by(dpos);
        extend_width = 0;
        extend_height = 0;
        extend_depth = 0;
        if (!eroom->map()->valid(eroom->start_pos)) {
            eroom->start_pos = {0,0,0};
        }
        eroom = editor_->reload(eroom);
    }

    static int shift_width;
    static int shift_height;
    static int shift_depth;

    ImGui::InputInt("Shift Width##SAVELOAD", &shift_width);
    ImGui::InputInt("Shift Height##SAVELOAD", &shift_height);
    ImGui::InputInt("Shift Depth##SAVELOAD", &shift_depth);

    clamp(&shift_width, 1 - cur_room_dims.x, MAX_ROOM_DIMS - cur_room_dims.x);
    clamp(&shift_height, 1 - cur_room_dims.y, MAX_ROOM_DIMS - cur_room_dims.y);
    clamp(&shift_depth, 1 - cur_room_dims.z, MAX_ROOM_DIMS - cur_room_dims.z);

    if (ImGui::Button("Shift room?##SAVELOAD")) {
        Point3 dpos = {shift_width, shift_height, shift_depth};
        eroom->room->shift_by(dpos);
        shift_width = 0;
        shift_height = 0;
        shift_depth = 0;
        eroom->start_pos += dpos;
        eroom->room->offset_pos_ += dpos;
        if (!eroom->map()->valid(eroom->start_pos)) {
            eroom->start_pos = {0,0,0};
        }
        eroom = editor_->reload(eroom);
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
