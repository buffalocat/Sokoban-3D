#include "editortab.h"

EditorTab::EditorTab(EditorState* editor, GraphicsManager* gfx): editor_ {editor}, gfx_ {gfx} {}

EditorTab::~EditorTab() {}

void EditorTab::init() {}

void EditorTab::handle_left_click(EditorRoom* eroom, Point3 pos) {}

void EditorTab::handle_right_click(EditorRoom* eroom, Point3 pos) {}
