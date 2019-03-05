#include "editortab.h"

#include "point.h"
#include "color_constants.h"

EditorTab::EditorTab(EditorState* editor, GraphicsManager* gfx): editor_ {editor}, gfx_ {gfx} {}

EditorTab::~EditorTab() {}

void EditorTab::init() {}

void EditorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {}

void EditorTab::handle_right_click(EditorRoom* eroom, Point3 pos) {}

void clamp(int* n, int a, int b) {
    *n = std::max(a, std::min(b, *n));
}

ImVec4 unpack_color(Color4 v) {
    return ImVec4(v.r, v.g, v.b, v.a);
}
