#include "objecttab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "common.h"
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
static bool block_push = true;
static bool block_grav = true;
static int sticky = (int)Sticky::None;
static int sb_ends = 2;

//static bool persistent = false;
//static bool switchable_state = true;


void ObjectTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Object Tab");

    ImGui::BeginChild("active layer pane##OBJECT", ImVec2(380, 450), true);

    ImGui::RadioButton("Wall##OBJECT_object", &obj_code, (int)ObjCode::Wall);
    ImGui::RadioButton("PushBlock##OBJECT_object", &obj_code, (int)ObjCode::PushBlock);
    ImGui::RadioButton("SnakeBlock##OBJECT_object", &obj_code, (int)ObjCode::SnakeBlock);

    switch (static_cast<ObjCode>(obj_code)) {
    /*
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
    */
    case ObjCode::PushBlock:
        ImGui::Checkbox("Pushable?##OBJECT_push", &block_push);
        ImGui::Checkbox("Gravitable?##OBJECT_grav", &block_grav);
        ImGui::InputInt("color##OBJECT_COLOR", &color);
        ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));
        ImGui::Text("Stickiness");
        ImGui::RadioButton("NonStick##OBJECT_sticky", &sticky, (int)Sticky::None);
        ImGui::RadioButton("Weakly Sticky##OBJECT_sticky", &sticky, (int)Sticky::Weak);
        ImGui::RadioButton("Strongly Sticky##OBJECT_sticky", &sticky, (int)Sticky::Strong);
        break;
    case ObjCode::SnakeBlock:
        ImGui::Checkbox("Pushable?##OBJECT_push", &block_push);
        ImGui::Checkbox("Gravitable?##OBJECT_grav", &block_grav);
        ImGui::InputInt("color##OBJECT_COLOR", &color);
        ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));
        ImGui::Text("Number of Ends");
        ImGui::RadioButton("One Ended##OBJECT_snake_ends", &sb_ends, 1);
        ImGui::RadioButton("Two Ended##OBJECT_snake_ends", &sb_ends, 2);
        break;
    }
    ImGui::EndChild();//*/
}

void ObjectTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->room->room_map();
    if (!room_map->valid(pos) || room_map->view(pos)) {
        return;
    }
    int x = pos.x;
    int y = pos.y;
    std::unique_ptr<GameObject> obj;
    switch (obj_code) {
    case (int)ObjCode::Wall :
        obj = std::make_unique<Wall>();
        break;
    case (int)ObjCode::PushBlock :
        obj = std::make_unique<PushBlock>(pos, color, block_push, block_grav, static_cast<Sticky>(sticky));
        break;
    case (int)ObjCode::SnakeBlock :
        obj = std::make_unique<SnakeBlock>(pos, color, block_push, block_grav, sb_ends);
        break;
    /*
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
    */
    default:
        return;
    }
    room_map->create(std::move(obj));
}

void ObjectTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->room->room_map();
    GameObject* obj = room_map->view(pos);
    if (obj) {
        if (obj->obj_code() == ObjCode::Player) {
            return;
        }
        room_map->remove_from_signalers(obj->modifier());
        room_map->take(obj);
    }
}
