#include "pushblock.h"

PushBlock::PushBlock(Point3 pos, int color, bool pushable, bool gravitable, Sticky sticky):
    GameObject(pos, color, pushable, gravitable), sticky_ {sticky} {}

void PushBlock::serialize(MapFileO& file) {
    file << color_;
}


void PushBlock::collect_sticky_links(RoomMap* room_map, Sticky sticky_level, std::vector<GameObject*>& links) {
    if (Sticky sticky_condition = sticky_ & sticky_level) {
        for (Point3 d : DIRECTIONS) {
            PushBlock* adj = dynamic_cast<PushBlock*>(room_map->view(cur->pos_ + d));
            if (adj && adj->color_ == color_ && (adj->sticky_ & sticky_condition)) {
                links.push_back(adj);
            }
        }
    }
}


/*
void Block::draw(GraphicsManager* gfx) {
    if (car_) {
        FPoint3 p {real_pos()};
        glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z + 0.5, p.y));
        model = glm::scale(model, glm::vec3(0.7f, 0.1f, 0.7f));
        gfx->set_model(model);
        gfx->set_color(COLORS[LIGHT_GREY]);
        gfx->draw_cube();
    }
}
*/
