#include "modifiertab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "common.h"

#include "gameobject.h"

#include "car.h"
#include "door.h"
#include "gate.h"
#include "pressswitch.h"

#include "colorcycle.h"

ModifierTab::ModifierTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

ModifierTab::~ModifierTab() {}

// Current object type
static ModCode mod_code = ModCode::NONE;
static bool inspect_mode = false;

// Model objects that new objects are created from
static Car model_car {nullptr, {}};
static Door model_door {nullptr, true, true};
static Gate model_gate {nullptr, nullptr, 0, false, false};
static PressSwitch model_press_switch {nullptr, 0, false, false};

static ColorCycle model_color_cycle {};

// Object Inspection
static GameObject* selected_obj = nullptr;

void ModifierTab::init() {
    selected_obj = nullptr;
}

void mod_tab_options();
void select_color_cycle(ColorCycle&);

void ModifierTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Modifier Tab");
    ImGui::Separator();
    if (!eroom) {
        ImGui::Text("No room loaded.");
        return;
    }

    ImGui::Checkbox("Inspect Mode##MOD_inspect", &inspect_mode);

    mod_tab_options();
}

void ModifierTab::mod_tab_options() {
    ObjectModifier* mod = nullptr;
    if (inspect_mode) {
        if (selected_obj) {
            mod = selected_obj->modifier();
            Point3 pos = selected_obj->pos_;
            ImGui::Text("Current selected object position: (%d,%d,%d)", pos.x, pos.y, pos.z);
            if (!mod) {
                ImGui::Text("It currently has no modifier.");
                return;
            }
        } else {
            ImGui::Text("No object selected!");
            return;
        }
    } else {
        ImGui::RadioButton("Car##MOD_object", &mod_code, ModCode::Car);
        ImGui::RadioButton("Door##MOD_object", &mod_code, ModCode::Door);
        ImGui::RadioButton("Gate##MOD_object", &mod_code, ModCode::Gate);
        ImGui::RadioButton("PressSwitch##MOD_object", &mod_code, ModCode::PressSwitch);
    }
    ImGui::Separator();
    switch (mod ? mod->mod_code() : mod_code) {
    case ModCode::Car:
        {
            ImGui::Text("Car");
            Car* car = mod ? static_cast<Car*>(mod) : &model_car;
            ColorCycle& color_cycle = car ? car->color_cycle_ : model_color_cycle;
            select_color_cycle(color_cycle);
        }
        break;
    case ModCode::Door:
        {
            ImGui::Text("Door");
            Door* door = mod ? static_cast<Door*>(mod) : &model_door;
            ImGui::Checkbox("Active by Default?##DOOR_default", &door->default_);
            door->active_ = door->default_;
        }
        break;
    case ModCode::Gate:
        {
            ImGui::Text("Gate");
            Gate* gate = mod ? static_cast<Gate*>(mod) : &model_gate;
            ImGui::Checkbox("Active by Default?##GATE_default", &gate->default_);
            gate->active_ = gate->default_;
            ImGui::InputInt("color##PRESS_SWITCH_modify_COLOR", &gate->color_);
            ImGui::ColorButton("##COLOR_BUTTON", unpack_color(COLORS[gate->color_]), 0, ImVec2(40,40));
        }
        break;
    case ModCode::PressSwitch:
        {
            ImGui::Text("PressSwitch");
            PressSwitch* ps = mod ? static_cast<PressSwitch*>(mod) : &model_press_switch;
            ImGui::Checkbox("Persistent?##PRESS_SWITCH_persistent", &ps->persistent_);
            ImGui::InputInt("color##PRESS_SWITCH_modify_COLOR", &ps->color_);
            ImGui::ColorButton("##COLOR_BUTTON", unpack_color(COLORS[ps->color_]), 0, ImVec2(40,40));
        }
        break;
    default:
        break;
    }
}

const char* color_ordinals[5] = {
    "First Color##COLOR_CYCLE_color",
    "Second Color##COLOR_CYCLE_color",
    "Third Color##COLOR_CYCLE_color",
    "Fourth Color##COLOR_CYCLE_color",
    "Fifth Color##COLOR_CYCLE_color",
};

void ModifierTab::select_color_cycle(ColorCycle& cycle) {
    ImGui::InputInt("Number of colors##COLOR_CYCLE_num", &cycle.size_);
    clamp(&cycle.size_, 1, MAX_COLOR_CYCLE);
    for (int i = 0; i < cycle.size_; ++i) {
        ImGui::InputInt(color_ordinals[i], &cycle.color_[i]);
        ImGui::ColorButton("##COLOR_BUTTON", unpack_color(COLORS[cycle.color_[i]]), 0, ImVec2(40,40));
    }
}


void ModifierTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    RoomMap* room_map = eroom->map();
    selected_obj = nullptr;
    if (!room_map->valid(pos)) {
        return;
    }
    GameObject* obj = room_map->view(pos);
    if (!obj) {
        return;
    } else if (inspect_mode || obj->modifier()) {
        selected_obj = obj;
        return;
    }
    std::unique_ptr<ObjectModifier> mod;
    switch (mod_code) {
    case ModCode::Car:
        mod = std::make_unique<Car>(model_car);
        break;
    case ModCode::Door:
        mod = std::make_unique<Door>(model_door);
        break;
    case ModCode::Gate:
        // TODO: handle gate body creation! somewhere!!!
        mod = std::make_unique<Gate>(model_gate);
        break;
    case ModCode::PressSwitch:
        mod = std::make_unique<PressSwitch>(model_press_switch);
        break;
    default:
        return;
    }
    mod->parent_ = obj;
    selected_obj = obj;
    obj->set_modifier(std::move(mod));
}

void ModifierTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    selected_obj = nullptr;
    // Don't allow deletion in inspect mode
    if (inspect_mode) {
        return;
    }
    RoomMap* room_map = eroom->map();
    if (GameObject* obj = room_map->view(pos)) {
        if (ObjectModifier* mod = obj->modifier()) {
            mod->cleanup_on_destruction(room_map);
            obj->set_modifier({});
        }
    }
}

