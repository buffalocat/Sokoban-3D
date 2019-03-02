#include "switchtab.h"

#include "gameobject.h"
#include "editorstate.h"
#include "room.h"
#include "roommap.h"
#include "switch.h"
#include "signaler.h"
#include "switchable.h"

#include <algorithm>

SwitchTab::SwitchTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx),
model_switches_ {}, model_switchables_ {} {}

SwitchTab::~SwitchTab() {}

static bool inspect_mode = false;
static Signaler* selected_sig = nullptr;

static std::vector<Switch*>* switches = nullptr;
static std::vector<Switchable*>* switchables = nullptr;

void SwitchTab::init() {
    model_switches_.clear();
    model_switchables_.clear();
    selected_sig = nullptr;
}

int SwitchTab::get_signaler_labels(const char* labels[], std::vector<std::unique_ptr<Signaler>>& signalers) {
    int i = 0;
    for (auto& s : signalers) {
        labels[i] = s->label_.c_str();
        ++i;
    }
    return i;
}


void SwitchTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Switch Tab");
    ImGui::Separator();
    if (!eroom) {
        ImGui::Text("No room loaded.");
        return;
    }
    ImGui::Checkbox("Inspect Mode##SWITCH_inspect", &inspect_mode);
    ImGui::Separator();

    if (inspect_mode) {
        static int current = 0;
        auto& signalers = eroom->map()->signalers_;
        const char* labels[1024];
        int len = get_signaler_labels(labels, signalers);
        if (ImGui::ListBox("Signalers##SWITCH", &current, labels, len, len)) {
            selected_sig = signalers[current].get();
        }
        if (selected_sig) {
            switches = &selected_sig->switches_;
            switchables = &selected_sig->switchables_;
        } else {
            ImGui::Text("No Signaler selected.");
            return;
        }
    } else {
        switches = &model_switches_;
        switchables = &model_switchables_;
    }

    ImGui::Text("Switches");
    for (int i = 0; i < switches->size(); ++i) {
        Switch* s = (*switches)[i];
        Point3 pos = s->pos();
        ImGui::Text("%s at (%d,%d,%d)", s->parent_->to_str().c_str(), pos.x, pos.y, pos.z);
        ImGui::SameLine();
        char buf[32];
        sprintf(buf, "Erase##SWITCH_a_%d", i);
        if (ImGui::Button(buf)) {
            switches->erase(std::remove(switches->begin(), switches->end(), s), switches->end());
        }
    }

    ImGui::Separator();

    ImGui::Text("Switchables");
    for (int i = 0; i < switchables->size(); ++i) {
        Switchable* s = (*switchables)[i];
        Point3 pos = s->pos();
        ImGui::Text("%s at (%d,%d,%d)", s->parent_->to_str().c_str(), pos.x, pos.y, pos.z);
        ImGui::SameLine();
        char buf[32];
        sprintf(buf, "Erase##SWITCH_b_%d", i);
        if (ImGui::Button(buf)) {
            switchables->erase(std::remove(switchables->begin(), switchables->end(), s), switchables->end());
        }
    }

    ImGui::Separator();

    if (!inspect_mode) {
        if (ImGui::Button("Empty Queued Objects##SWITCH")) {
            model_switches_.clear();
            model_switchables_.clear();
        }
    }

    const static int MAX_LABEL_LENGTH = 64;
    static char label_buf[MAX_LABEL_LENGTH] = "";
    if (inspect_mode) {
        if (selected_sig) {
            snprintf(label_buf, MAX_LABEL_LENGTH, "%s", selected_sig->label_.c_str());
            if (ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH)) {
                selected_sig->label_ = std::string(label_buf);
            }
        }
    } else {
        ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH);
    }

    if (inspect_mode) {
        if (ImGui::Button("Erase Selected Signaler##SWITCH")) {
            eroom->map()->remove_signaler(selected_sig);
            selected_sig = nullptr;
        }
        return;
    }

    if (model_switchables_.size() && model_switches_.size()) {
        if (ImGui::Button("Make Signaler##SWITCH")) {
            std::string label {label_buf};
            if (label.empty()) {
                label = "UNNAMED";
            }
            auto signaler = std::make_unique<Signaler>(label, 0, model_switches_.size(), false, false);
            for (auto& obj : model_switches_) {
                signaler->push_switch_mutual(obj);
            }
            for (auto& obj : model_switchables_) {
                signaler->push_switchable_mutual(obj);
            }
            eroom->map()->push_signaler(std::move(signaler));
            model_switches_.clear();
            model_switchables_.clear();
        }
    }
}

void SwitchTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    if (!switches || !switchables) {
        return;
    }
    if (GameObject* obj = eroom->map()->view(pos)) {
        if (ObjectModifier* mod = obj->modifier()) {
            auto swble = dynamic_cast<Switchable*>(mod);
            if (swble && std::find(switchables->begin(), switchables->end(), swble) == switchables->end()) {
                switchables->push_back(swble);
                return;
            }
            auto sw = dynamic_cast<Switch*>(mod);
            if (sw && std::find(switches->begin(), switches->end(), sw) == switches->end())  {
                switches->push_back(sw);
                return;
            }
        }
    }
}

void SwitchTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    if (!switches || !switchables) {
        return;
    }
    if (GameObject* obj = eroom->map()->view(pos)) {
        ObjectModifier* mod = obj->modifier();
        switches->erase(std::remove(switches->begin(), switches->end(), mod), switches->end());
        switchables->erase(std::remove(switchables->begin(), switchables->end(), mod), switchables->end());
    }
}
