#include "editortab.h"

EditorTab::EditorTab(EditorState* editor, GraphicsManager* gfx): editor_ {editor}, gfx_ {gfx} {}

EditorTab::~EditorTab() {}

void EditorTab::init() {}

void EditorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {}

void EditorTab::handle_right_click(EditorRoom* eroom, Point3 pos) {}

ImVec4 unpack_color(glm::vec4 v) {
    return ImVec4(v.x, v.y, v.z, v.w);
}
