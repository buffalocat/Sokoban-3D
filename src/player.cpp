#include "player.h"
#include "pushblock.h"

#include "delta.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "mapfile.h"

Player::Player(Point3 pos, RidingState state): PushBlock(pos, PINK, true, true, Sticky::None), state_ {state} {}

Player::~Player() {}

ObjCode Player::obj_code() const {
    return ObjCode::Player;
}

void Player::set_riding(RidingState state) {
    state_ = state;
}

RidingState Player::state() {
    return state_;
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
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z - 0.5f * (state_ == RidingState::Riding), p.y));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    gfx->set_model(model);
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
}

void Player::serialize(MapFileO& file) {
    file << state_;
}

GameObject* Player::deserialize(MapFileI& file) {
    unsigned char b[4];
    file.read(b, 4);
    return new Player(Deser::p3(b), static_cast<RidingState>(b[3]));
}

// NOTE: if the Player becomes a subclass of a more general "Passenger" type, move this up to that class.
void Player::collect_special_links(RoomMap* room_map, Sticky sticky_level, std::vector<GameObject*>& links) const {
    if (state_ == RidingState::Riding) {
        links.push_back(room_map->view(shifted_pos({0,0,-1})));
    }
}
