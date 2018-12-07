#include "effects.h"
#include "graphicsmanager.h"
#include "block.h"

#include <algorithm>

Effects::Effects(): trails_ {} {}

Effects::~Effects() {}

const unsigned int FALL_TRAIL_OPACITY = 8;
const float MAX_OPACITY = 10.0;

bool is_zero_opacity(FallTrail trail) {
    return trail.opacity == 0;
}

void Effects::sort_by_distance(float angle) {}

void Effects::draw(GraphicsManager* gfx) {
    for (auto& trail : trails_) {
        glm::vec4 color = COLORS[trail.color];
        color.w = trail.opacity/MAX_OPACITY;
        --trail.opacity;
        gfx->set_color(color);
        Point3 base = trail.base;
        glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(base.x, base.z, base.y));
        model = glm::scale(model, glm::vec3(0.3,trail.height,0.3));
        model = glm::translate(model, glm::vec3(0,0.5,0));
        gfx->set_model(model);
        gfx->draw_cube();
    }
    trails_.erase(std::remove_if(trails_.begin(), trails_.end(), &is_zero_opacity), trails_.end());
}

void Effects::push_trail(Block* block, int height, int drop) {
    trails_.push_back({block->pos() - Point3{0,0,drop}, height+drop, FALL_TRAIL_OPACITY, block->color()});
}
