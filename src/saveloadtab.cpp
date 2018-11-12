#include "saveloadtab.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <dear/imgui.h>
#pragma GCC diagnostic pop

#include "room.h"
#include "roommap.h"
#include "gameobject.h"

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
