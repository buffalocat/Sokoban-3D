#include "snaketab.h"

#include "editorstate.h"
#include "room.h"
#include "roommap.h"

#include "common.h"

#include "gameobject.h"
#include "snakeblock.h"

SnakeTab::SnakeTab(EditorState* editor, GraphicsManager* gfx): EditorTab(editor, gfx) {}

SnakeTab::~SnakeTab() {}


static SnakeBlock* selected_snake_a = nullptr;
static SnakeBlock* selected_snake_b = nullptr;
static bool delete_mode = false;

void SnakeTab::init() {
    selected_snake_a = nullptr;
    selected_snake_b = nullptr;
}


void SnakeTab::main_loop(EditorRoom* eroom) {
    ImGui::Text("The Snake Tab");
    ImGui::Separator();
    if (!eroom) {
        ImGui::Text("No room loaded.");
        return;
    }

    ImGui::Text("Left Click Snakes to create links.");
    ImGui::Text("Right Click Snakes to destroy links.");
    ImGui::Separator();
    if (ImGui::Button("Form Automatic Links##SNAKE")) {
        eroom->map()->initialize_automatic_snake_links();
    }
    ImGui::Separator();

    editor_->display_hover_pos_object(eroom->cam_pos, eroom->map());

    ImGui::Separator();

    if (selected_snake_b) {
        Point3 p = selected_snake_b->pos_;
        ImGui::Text("First Snake at (%d,%d,%d)", p.x, p.y, p.z);
        if (selected_snake_a) {
            p = selected_snake_a->pos_;
            ImGui::Text("Second Snake at (%d,%d,%d)", p.x, p.y, p.z);
        }
    }
}

void SnakeTab::handle_left_click(EditorRoom* eroom, Point3 pos) {
    if (delete_mode) {
        delete_mode = false;
        selected_snake_a = nullptr;
        selected_snake_b = nullptr;
    }
    handle_click_generic(eroom, pos);
    if (selected_snake_a && selected_snake_b && selected_snake_a->can_link(selected_snake_b)) {
        selected_snake_a->add_link_quiet(selected_snake_b);
    }
}

void SnakeTab::handle_right_click(EditorRoom* eroom, Point3 pos) {
    if (!delete_mode) {
        delete_mode = true;
        selected_snake_a = nullptr;
        selected_snake_b = nullptr;
    }
    handle_click_generic(eroom, pos);
    if (selected_snake_a && selected_snake_b) {
        if (selected_snake_a->in_links(selected_snake_b)) {
            selected_snake_a->remove_link_quiet(selected_snake_b);
        }
    }
}

void SnakeTab::handle_click_generic(EditorRoom* eroom, Point3 pos) {
    if (SnakeBlock* snake = dynamic_cast<SnakeBlock*>(eroom->map()->view(pos))) {
        if (snake != selected_snake_b) {
            selected_snake_a = selected_snake_b;
        }
        selected_snake_b = snake;
    } else {
        selected_snake_a = nullptr;
        selected_snake_b = nullptr;
    }
}
