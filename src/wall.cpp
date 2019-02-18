#include "wall.h"

#include "graphicsmanager.h"

Wall::Wall(): PushBlock({0,0,0}, 0, false, false, Sticky::None) {}

Wall::~Wall() {}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

void Wall::draw(GraphicsManager* gfx, Point3 p) {
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(GREYS[p.z % NUM_GREYS]);
    gfx->draw_cube();
}

GameObject* Wall::deserialize(MapFileI& file) {
    return new Wall();
}
