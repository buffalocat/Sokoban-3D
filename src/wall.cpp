#include "wall.h"

#include "graphicsmanager.h"

Wall::Wall(): PushBlock({0,0,0}, 0, false, false, Sticky::None) {}

Wall::~Wall() {}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

// NOTE: Defining this is redundant, as it's not possible to create
// a Wall whose ID isn't GLOBAL_WALL_ID (at least without save hacking)
bool Wall::skip_serialization() {
    return true;
}

// TODO: Replace with a batch drawing mechanism!
void Wall::draw(GraphicsManager* gfx) {}
