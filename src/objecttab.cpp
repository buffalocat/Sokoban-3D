#include "objecttab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "common.h"

#include "gameobject.h"
#include "pushblock.h"
#include "snakeblock.h"
#include "wall.h"
#include "gatebody.h"
#include "gate.h"

ObjectTab::ObjectTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

ObjectTab::~ObjectTab() {}

// Current object type
static ObjCode obj_code = ObjCode::NONE;
static bool inspect_mode = false;

// Model objects that new objects are created from
static PushBlock model_pb {Point3{0,0,0}, 0, true, true, Sticky::None};
static SnakeBlock model_sb {Point3{0,0,0}, 0, true, true, 2};

// Object Inspection
static GameObject* selected_obj = nullptr;

void object_tab_options();

void ObjectTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Object Tab");
    ImGui::Separator();

    ImGui::Checkbox("Inspect Mode##OBJECT_inspect", &inspect_mode);

    object_tab_options();
}

void object_tab_options() {
    GameObject* obj = nullptr;
    if (inspect_mode) {
        if (selected_obj) {
            obj = selected_obj;
            Point3 pos = obj->pos_;
            if (obj->obj_code() == ObjCode::Wall) {
                ImGui::Text("Wall selected");
                return;
            } else {
                ImGui::Text("Current selected object position: (%d,%d,%d)", pos.x, pos.y, pos.z);
            }
        } else {
            ImGui::Text("No object selected!");
            return;
        }
    } else {
        ImGui::RadioButton("Wall##OBJECT_object", &obj_code, ObjCode::Wall);
        ImGui::RadioButton("PushBlock##OBJECT_object", &obj_code, ObjCode::PushBlock);
        ImGui::RadioButton("SnakeBlock##OBJECT_object", &obj_code, ObjCode::SnakeBlock);
    }
    ImGui::Separator();
    switch (obj ? obj->obj_code() : obj_code) {
    case ObjCode::PushBlock:
        {
            ImGui::Text("PushBlock");
            PushBlock* pb = obj ? static_cast<PushBlock*>(obj) : &model_pb;
            ImGui::Checkbox("Pushable?##PB_modify_push", &pb->pushable_);
            ImGui::Checkbox("Gravitable?##PB_modify_grav", &pb->gravitable_);
            ImGui::InputInt("color##PB_modify_COLOR", &pb->color_);
            ImGui::ColorButton("##COLOR_BUTTON", unpack_color(COLORS[pb->color_]), 0, ImVec2(40,40));
            ImGui::Text("Stickiness");
            ImGui::RadioButton("NonStick##PB_modify_sticky", &pb->sticky_, Sticky::None);
            ImGui::RadioButton("Weakly Sticky##PB_modify_sticky", &pb->sticky_, Sticky::Weak);
            ImGui::RadioButton("Strongly Sticky##PB_modify_sticky", &pb->sticky_, Sticky::Strong);
        }
        break;
    case ObjCode::SnakeBlock:
        {
            ImGui::Text("SnakeBlock");
            SnakeBlock* sb = obj ? static_cast<SnakeBlock*>(obj) : &model_sb;
            ImGui::Checkbox("Pushable?##SB_modify_push", &sb->pushable_);
            ImGui::Checkbox("Gravitable?##SB_modify_grav", &sb->gravitable_);
            ImGui::InputInt("color##SB_modify_COLOR", &sb->color_);
            ImGui::ColorButton("##COLOR_BUTTON", unpack_color(COLORS[sb->color_]), 0, ImVec2(40,40));
            ImGui::Text("Number of Ends");
            ImGui::RadioButton("One Ended##SB_modify_snake_ends", &sb->ends_, 1);
            ImGui::RadioButton("Two Ended##SB_modify_snake_ends", &sb->ends_, 2);
        }
        break;
    case ObjCode::GateBody:
        {
            ImGui::Text("GateBody");
            Point3 b_pos = static_cast<GateBody*>(obj)->base_->pos();
            ImGui::Text("See parent Gate at (%d,%d,%d)", b_pos.x, b_pos.y, b_pos.z);
        }
    case ObjCode::Wall: // No parameters for Wall
    default:
        break;
    }
}

void ObjectTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->room->room_map();
    if (!room_map->valid(pos)) {
        selected_obj = nullptr;
        return;
    // Even in create mode, let the user select an already-created object
    } else if (inspect_mode || room_map->view(pos)) {
        selected_obj = room_map->view(pos);
        return;
    }
    std::unique_ptr<GameObject> obj;
    switch (obj_code) {
    case ObjCode::Wall :
        room_map->create_wall(pos);
        return;
    case ObjCode::PushBlock :
        obj = std::make_unique<PushBlock>(model_pb);
        break;
    case ObjCode::SnakeBlock :
        obj = std::make_unique<SnakeBlock>(model_sb);
        break;
    default:
        return;
    }
    obj->pos_ = pos;
    selected_obj = obj.get();
    room_map->create(std::move(obj));
}

void ObjectTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    // Don't allow deletion in inspect mode
    if (inspect_mode) {
        return;
    }
    RoomMap* room_map = eroom->room->room_map();
    GameObject* obj = room_map->view(pos);
    if (obj) {
        if (obj->obj_code() == ObjCode::Player) {
            return;
        }
        selected_obj = nullptr;
        room_map->remove_from_signalers(obj->modifier());
        // When we "destroy" a wall, it doesn't actually destroy the unique Wall object
        room_map->destroy(obj);
    }
}
