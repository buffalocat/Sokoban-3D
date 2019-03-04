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
model_switches_ {}, model_switchables_ {}, model_persistent_ {}, model_threshold_ {} {}

SwitchTab::~SwitchTab() {}

static bool inspect_mode = false;
static Signaler* selected_sig = nullptr;
static bool* persistent = nullptr;
static int* threshold = nullptr;

static std::vector<Switch*>* switches = nullptr;
static std::vector<Switchable*>* switchables = nullptr;

enum class Threshold {
    All,
    Any,
    Custom,
};

static Threshold threshold_mode = Threshold::All;

void SwitchTab::init() {
    model_switches_.clear();
    model_switchables_.clear();
    model_persistent_ = false;
    model_threshold_ = 0;
    threshold_mode = Threshold::All;
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
            int cur = selected_sig->threshold_;
            if (cur == selected_sig->switches_.size()) {
                threshold_mode = Threshold::All;
            } else if (cur == 1) {
                threshold_mode = Threshold::Any;
            } else {
                threshold_mode = Threshold::Custom;
            }
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

    persistent = &model_persistent_;
    threshold = &model_threshold_;
    if (inspect_mode) {
        if (selected_sig) {
            snprintf(label_buf, MAX_LABEL_LENGTH, "%s", selected_sig->label_.c_str());
            if (ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH)) {
                selected_sig->label_ = std::string(label_buf);
            }
            persistent = &selected_sig->persistent_;
            threshold = &selected_sig->threshold_;
        } else {
            return;
        }
    } else {
        ImGui::InputText("Label##SWITCH_signaler_label", label_buf, MAX_LABEL_LENGTH);
        persistent = &model_persistent_;
        threshold = &model_threshold_;
    }

    ImGui::Checkbox("Persistent?##SWITCH_persistent", persistent);

    ImGui::Separator();
    ImGui::Text("Activation Threshold:");

    ImGui::RadioButton("All##SWITCH_threshold", &threshold_mode, Threshold::All);
    ImGui::RadioButton("Any##SWITCH_threshold", &threshold_mode, Threshold::Any);
    ImGui::RadioButton("Custom##SWITCH_threshold", &threshold_mode, Threshold::Custom);

    switch (threshold_mode) {
    case Threshold::All:
        *threshold = switches->size();
        break;
    case Threshold::Any:
        *threshold = 1;
        break;
    case Threshold::Custom:
        ImGui::InputInt("Switch Threshold##SWITCH_threshold", threshold);
        if (*threshold < 0) {
            *threshold = 0;
        }
        break;
    }

    ImGui::Separator();

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
            auto signaler = std::make_unique<Signaler>(label, 0, *threshold, *persistent, false);
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
