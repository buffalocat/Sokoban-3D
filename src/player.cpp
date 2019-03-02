#include "player.h"
#include "pushblock.h"

#include "delta.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "mapfile.h"

Player::Player(Point3 pos, RidingState state): PushBlock(pos, PINK, true, true, Sticky::None), state_ {state} {}

Player::~Player() {}

std::string Player::name() {
    return "Player";
}

ObjCode Player::obj_code() {
    return ObjCode::Player;
}

bool Player::skip_serialization() {
    return true;
}

void Player::toggle_riding(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (state_ == RidingState::Riding) {
        delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
        state_ = RidingState::Bound;
    } else if (state_ == RidingState::Bound) {
        if (dynamic_cast<Car*>(room_map->view(shifted_pos({0,0,-1}))->modifier())) {
            delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
            state_ = RidingState::Riding;
        }
    }
}

Car* Player::get_car(RoomMap* room_map, bool strict) {
    if (state_ == RidingState::Free || (strict && state_ == RidingState::Bound)) {
        return nullptr;
    } else {
        //NOTE: if there are no bugs elsewhere, this could be a static cast
        return dynamic_cast<Car*>(room_map->view(shifted_pos({0,0,-1}))->modifier());
    }
}

void Player::draw(GraphicsManager* gfx) {
    FPoint3 p = real_pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z - 0.7f * (state_ == RidingState::Riding), p.y));
    model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
    gfx->set_model(model);
    gfx->set_tex(Texture::Blank);
    switch (state_) {
    case RidingState::Free:
        gfx->set_color(COLORS[BLUE]);
        break;
    case RidingState::Bound:
        gfx->set_color(COLORS[PINK]);
        break;
    case RidingState::Riding:
        gfx->set_color(COLORS[RED]);
        break;
    }
    gfx->draw_cube();
    if (modifier_) {
        modifier()->draw(gfx, p);
    }
}

/*std::unique_ptr<GameObject> Player::deserialize(MapFileI& file) {
    Point3 pos = file.read_point3();
    RidingState state = static_cast<RidingState>(file.read_byte());
    return std::make_unique<Player>(pos, state);
}*/

// NOTE: if the Player becomes a subclass of a more general "Passenger" type, move this up to that class.
void Player::collect_special_links(RoomMap* room_map, Sticky sticky_level, std::vector<GameObject*>& links) {
    if (state_ == RidingState::Riding) {
        links.push_back(room_map->view(shifted_pos({0,0,-1})));
    }
}
