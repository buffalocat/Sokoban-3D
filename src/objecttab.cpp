#include "objecttab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"


#include "switch.h"
#include "door.h"
#include "snakeblock.h"
#include "gameobject.h"
#include "colorcycle.h"
#include "wall.h"

ObjectTab::ObjectTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

ObjectTab::~ObjectTab() {}

ImVec4 unpack_color(glm::vec4 v) {
    return ImVec4(v.x, v.y, v.z, v.w);
}

// Object creation variables
static int obj_code = (int)ObjCode::NONE;

static int color = GREEN;
static bool two_colors = false;
static int alt_color = PINK;
static bool is_car = false;
static int sb_ends = 2;
static bool persistent = false;
static bool switchable_state = true;

void block_options() {
    ImGui::InputInt("color##OBJECT_COLOR", &color);
    ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));
    ImGui::Checkbox("Is Rideable?##OBJECT_block", &is_car);
    ImGui::Checkbox("Second Color?##OBJECT_block", &two_colors);
    if (two_colors) {
        ImGui::InputInt("color##OBJECT_COLOR_2", &alt_color);
        ImGui::ColorButton("##OBJECT_COLOR_BUTTON_2", unpack_color(COLORS[alt_color]), 0, ImVec2(40,40));
    }
}

void ObjectTab::main_loop(EditorRoom* eroom) {
    /*
    ImGui::Text("The Object Tab");

    ImGui::BeginChild("active layer pane##OBJECT", ImVec2(380, 450), true);

    ImGui::RadioButton("Door##OBJECT_object", &obj_code, (int)ObjCode::Door);
    ImGui::RadioButton("PressSwitch##OBJECT_object", &obj_code, (int)ObjCode::PressSwitch);
    ImGui::RadioButton("Gate##OBJECT_object", &obj_code, (int)ObjCode::Gate);
    ImGui::RadioButton("Wall##OBJECT_object", &obj_code, (int)ObjCode::Wall);
    ImGui::RadioButton("NonStickBlock##OBJECT_object", &obj_code, (int)ObjCode::NonStickBlock);
    ImGui::RadioButton("WeakBlock##OBJECT_object", &obj_code, (int)ObjCode::WeakBlock);
    ImGui::RadioButton("StickyBlock##OBJECT_object", &obj_code, (int)ObjCode::StickyBlock);
    ImGui::RadioButton("SnakeBlock##OBJECT_object", &obj_code, (int)ObjCode::SnakeBlock);

    switch (static_cast<ObjCode>(obj_code)) {
    case ObjCode::Door:
        ImGui::Checkbox("default##SWITCHABLE", &switchable_state);
        break;
    case ObjCode::PressSwitch:
        ImGui::InputInt("color##OBJECT_COLOR", &color);
        ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));
        ImGui::Checkbox("persistent##SWITCH", &persistent);
        break;
    case ObjCode::Gate:
        ImGui::Checkbox("default##SWITCHABLE", &switchable_state);
        break;
    case ObjCode::NonStickBlock: // Fallthrough
    case ObjCode::WeakBlock:
    case ObjCode::StickyBlock:
        block_options();
        break;
    case ObjCode::SnakeBlock:
        ImGui::InputInt("color##OBJECT_COLOR", &color);
        ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));

        ImGui::Text("Number of Ends");
        ImGui::RadioButton("One Ended##OBJECT_snake_ends", &sb_ends, 1);
        ImGui::RadioButton("Two Ended##OBJECT_snake_ends", &sb_ends, 2);

        ImGui::Checkbox("Is Rideable?##OBJECT_car", &is_car);
        break;
    }
    ImGui::EndChild();//*/
}

void ObjectTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->room->room_map();
    if (room_map->view(pos)) {
        return;
    }
    int x = pos.x;
    int y = pos.y;
    std::unique_ptr<GameObject> obj;
    // TODO: FIX OBJECT CREATION
    /*
    switch (obj_code) {
    case (int)ObjCode::Wall :
        obj = std::make_unique<Wall>();
        break;
    case (int)ObjCode::NonStickBlock :
        obj = std::make_unique<NonStickBlock>(pos, color_cycle, is_car);
        break;
    case (int)ObjCode::WeakBlock :
        obj = std::make_unique<WeakBlock>(pos, color_cycle, is_car);
        break;
    case (int)ObjCode::StickyBlock :
        obj = std::make_unique<StickyBlock>(pos, color_cycle, is_car);
        break;
    case (int)ObjCode::SnakeBlock :
        obj = std::make_unique<SnakeBlock>(pos, color_cycle, is_car, sb_ends);
        break;
    case (int)ObjCode::Door :
        obj = std::make_unique<Door>(pos, switchable_state);
        break;
    case (int)ObjCode::PressSwitch :
        obj = std::make_unique<PressSwitch>(pos, color, persistent, false);
        break;
    case (int)ObjCode::Gate :
        obj = std::make_unique<Gate>(pos, switchable_state);
        break;
    default:
        return;
    }
    */
    room_map->create(std::move(obj), nullptr);
}

void ObjectTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->room->room_map();
    GameObject* obj = room_map->view(pos);
    if (obj) {
        if (obj->obj_code() == ObjCode::Player) {
            return;
        }
        room_map->remove_from_signalers(obj);
        room_map->take(obj);
    }
}
