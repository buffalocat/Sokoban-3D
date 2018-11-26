#include "player.h"

#include "delta.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "mapfile.h"

Player::Player(Point3 pos, RidingState state): Block(pos, 0, false), state_ {state} {}

Player::~Player() {}

ObjCode Player::obj_code() {
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
        Block* block = dynamic_cast<Block*>(room_map->view(shifted_pos({0,0,-1})));
        if (block && block->car()) {
            delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
            state_ = RidingState::Riding;
        }
    }
}

Block* Player::get_car(RoomMap* room_map, bool strict) {
    if (state_ == RidingState::Free || (strict && state_ == RidingState::Bound)) {
        return nullptr;
    } else {
        GameObject* car = room_map->view(shifted_pos({0,0,-1}));
        return static_cast<Block*>(car);
    }
}

void Player::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z + 0.5f * (state_ == RidingState::Bound), p.y));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    gfx->set_model(model);
    gfx->set_color(COLORS[PINK]);
    gfx->draw_cube();
}

void Player::serialize(MapFileO& file) {
    file << state_;
}

GameObject* Player::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[1];
    file.read(b,1);
    return new Player(pos, static_cast<RidingState>(b[0]));
}
