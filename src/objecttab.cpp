#include "objecttab.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <dear/imgui.h>
#pragma GCC diagnostic pop

#include "room.h"
#include "roommap.h"

#include "block.h"
#include "switch.h"
#include "door.h"

ObjectTab::ObjectTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
layer {(int)Layer::Solid}, obj_code {(int)ObjCode::NONE},
color {GREEN}, pb_sticky {(int)StickyLevel::None},
is_car {true}, sb_ends {2}, persistent {false}, default_state {false} {}

ImVec4 unpack_color(glm::vec4 v) {
    return ImVec4(v.x, v.y, v.z, v.w);
}

void ObjectTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Object Tab");

    ImGui::RadioButton("Floor##OBJECT_layer", &layer, (int)Layer::Floor);
    ImGui::RadioButton("Player##OBJECT_layer", &layer, (int)Layer::Player);
    ImGui::RadioButton("Solid##OBJECT_layer", &layer, (int)Layer::Solid);

    ImGui::BeginChild("active layer pane##OBJECT", ImVec2(380, 450), true);

    if (layer == (int)Layer::Floor) {
        ImGui::RadioButton("Door##OBJECT_object", &obj_code, (int)ObjCode::Door);
        ImGui::RadioButton("PressSwitch##OBJECT_object", &obj_code, (int)ObjCode::PressSwitch);
        ImGui::RadioButton("Gate##OBJECT_object", &obj_code, (int)ObjCode::Gate);

        if (obj_code == (int)ObjCode::Door) {}
        else if (obj_code == (int)ObjCode::PressSwitch) {
            ImGui::InputInt("color##OBJECT_COLOR", &color);
            ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));
            ImGui::Checkbox("persistent##SWITCH", &persistent);
        } else if (obj_code == (int)ObjCode::Gate) {
            ImGui::Checkbox("default##GATE", &default_state);
        }
    } else if (layer == (int)Layer::Player) {
        ImGui::RadioButton("PlayerWall##OBJECT_object", &obj_code, (int)ObjCode::PlayerWall);
    } else if (layer == (int)Layer::Solid) {
        ImGui::RadioButton("Wall##OBJECT_object", &obj_code, (int)ObjCode::Wall);
        ImGui::RadioButton("PushBlock##OBJECT_object", &obj_code, (int)ObjCode::PushBlock);
        ImGui::RadioButton("SnakeBlock##OBJECT_object", &obj_code, (int)ObjCode::SnakeBlock);

        ImGui::BeginChild("active solid object pane", ImVec2(360, 300), true);

        if (obj_code == (int)ObjCode::Wall) {}
        else if (obj_code == (int)ObjCode::PushBlock) {
            ImGui::InputInt("color##OBJECT_COLOR", &color);
            ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));

            ImGui::Text("Stickiness");
            ImGui::RadioButton("Not Sticky##OBJECT_sticky", &pb_sticky, (int)StickyLevel::None);
            ImGui::RadioButton("Weakly Sticky##OBJECT_sticky", &pb_sticky, (int)StickyLevel::Weak);
            ImGui::RadioButton("Strongly Sticky##OBJECT_sticky", &pb_sticky, (int)StickyLevel::Strong);

            ImGui::Checkbox("Is Rideable?##OBJECT_car", &is_car);
        }
        else if (obj_code == (int)ObjCode::SnakeBlock) {
            ImGui::InputInt("color##OBJECT_COLOR", &color);
            ImGui::ColorButton("##OBJECT_COLOR_BUTTON", unpack_color(COLORS[color]), 0, ImVec2(40,40));

            ImGui::Text("Number of Ends");
            ImGui::RadioButton("One Ended##OBJECT_snake_ends", &sb_ends, 1);
            ImGui::RadioButton("Two Ended##OBJECT_snake_ends", &sb_ends, 2);

            ImGui::Checkbox("Is Rideable?##OBJECT_car", &is_car);
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void ObjectTab::handle_left_click(EditorRoom* eroom, Point pos) {
    RoomMap* room_map = eroom->room->room_map();
    if (room_map->view(pos, static_cast<Layer>(layer))) {
        return;
    }
    int x = pos.x;
    int y = pos.y;
    std::unique_ptr<GameObject> obj;
    switch (obj_code) {
    case (int)ObjCode::Wall :
        obj = std::make_unique<Wall>(x, y);
        break;
    case (int)ObjCode::PushBlock :
        obj = std::make_unique<PushBlock>(x, y, color, is_car, static_cast<StickyLevel>(pb_sticky));
        break;
    case (int)ObjCode::SnakeBlock :
        obj = std::make_unique<SnakeBlock>(x, y, color, is_car, sb_ends);
        break;
    case (int)ObjCode::PlayerWall :
        obj = std::make_unique<PlayerWall>(x, y);
        break;
    case (int)ObjCode::Door :
        obj = std::make_unique<Door>(x, y);
        break;
    case (int)ObjCode::PressSwitch :
        obj = std::make_unique<PressSwitch>(x, y, color, persistent, false);
        break;
    case (int)ObjCode::Gate :
        obj = std::make_unique<Gate>(x, y, default_state);
        break;
    default:
        return;
    }
    if (layer == (int)obj->layer()) {
        Block* block = dynamic_cast<Block*>(obj.get());
        room_map->put_quiet(std::move(obj));
        if (block) {
            block->check_add_local_links(room_map, nullptr);
        }
    }
}

void ObjectTab::handle_right_click(EditorRoom* eroom, Point pos) {
    RoomMap* room_map = eroom->room->room_map();
    GameObject* obj = room_map->view(pos, static_cast<Layer>(layer));
    if (obj) {
        if (obj->obj_code() == ObjCode::Player) {
            return;
        }
        obj->cleanup(nullptr);
        auto sb = dynamic_cast<SnakeBlock*>(obj);
        room_map->take_quiet(obj);
        if (sb) {
            for (auto& d : DIRECTIONS) {
                auto snake = dynamic_cast<SnakeBlock*>(room_map->view(Point{pos.x + d.x, pos.y + d.y}, Layer::Solid));
                if (snake) {
                    snake->check_add_local_links(room_map, nullptr);
                }
            }
        }
    }
}
