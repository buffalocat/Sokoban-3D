#include "common.h"

#include "gameobject.h"
#include "block.h"
#include "roommap.h"
#include "graphicsmanager.h"
#include "delta.h"


GameObject::GameObject(int x, int y): pos_ {x, y} {}

GameObject::~GameObject() {}

void GameObject::serialize(std::ofstream& file) {}

bool GameObject::relation_check() {
    return false;
}

void GameObject::relation_serialize(std::ofstream& file) {}

Point GameObject::pos() const {
    return pos_;
}

Point GameObject::shifted_pos(Point d) const {
    return Point{pos_.x + d.x, pos_.y + d.y};
}

void GameObject::shift_pos_auto(Point d, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto self_unique = room_map->take_quiet(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_, room_map));
    }
    pos_.x += d.x;
    pos_.y += d.y;
    room_map->put_quiet(std::move(self_unique));
}

void GameObject::set_pos(Point p) {
    pos_ = p;
}

void GameObject::set_pos_auto(Point p, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto self_unique = room_map->take_quiet(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_, room_map));
    }
    pos_ = p;
    room_map->put_quiet(std::move(self_unique));
}

void GameObject::reinit() {}

void GameObject::cleanup(DeltaFrame* delta_frame) {}

Wall::Wall(int x, int y): GameObject(x, y) {}

Wall::~Wall() {}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

Layer Wall::layer() {
    return Layer::Solid;
}

void Wall::draw(GraphicsManager* gfx) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f, p.y));
    gfx->set_model(model);
    gfx->set_color(COLORS[BLACK]);
    gfx->draw_cube();
}

GameObject* Wall::deserialize(unsigned char* b) {
    return new Wall(b[0], b[1]);
}

Player::Player(int x, int y, RidingState state): GameObject(x, y), state_ {state} {}

Player::~Player() {}

ObjCode Player::obj_code() {
    return ObjCode::Player;
}

Layer Player::layer() {
    return Layer::Player;
}

RidingState Player::state() {
    return state_;
}

void Player::toggle_riding(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (state_ == RidingState::Riding) {
        delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
        state_ = RidingState::Bound;
    } else if (state_ == RidingState::Bound) {
        Block* block = dynamic_cast<Block*>(room_map->view(pos(), Layer::Solid));
        if (block && block->is_car()) {
            delta_frame->push(std::make_unique<RidingStateDelta>(this, state_));
            state_ = RidingState::Riding;
        }
    }
}

void Player::set_riding(RidingState state) {
    state_ = state;
}

Block* Player::get_car(RoomMap* room_map) {
    if (state_ != RidingState::Riding) {
        return nullptr;
    } else {
        GameObject* car = room_map->view(pos_, Layer::Solid);
        return static_cast<Block*>(car);
    }
}

void Player::draw(GraphicsManager* gfx) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 1.0f + 0.5f * (state_ == RidingState::Bound), p.y));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    gfx->set_model(model);
    gfx->set_color(COLORS[PINK]);
    gfx->draw_cube();
}

void Player::serialize(std::ofstream& file) {
    file << static_cast<unsigned char>(state_);
}

GameObject* Player::deserialize(unsigned char* b) {
    return new Player(b[0], b[1], static_cast<RidingState>(b[2]));
}

PlayerWall::PlayerWall(int x, int y): GameObject(x, y) {}

PlayerWall::~PlayerWall() {}

ObjCode PlayerWall::obj_code() {
    return ObjCode::PlayerWall;
}

Layer PlayerWall::layer() {
    return Layer::Player;
}

void PlayerWall::draw(GraphicsManager* gfx) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 1.1f, p.y));
    model = glm::scale(model, glm::vec3(0.5f, 0.2f, 0.5f));
    gfx->set_model(model);
    gfx->set_color(COLORS[BLACK]);
    gfx->draw_cube();
}

GameObject* PlayerWall::deserialize(unsigned char* b) {
    return new PlayerWall(b[0], b[1]);
}

