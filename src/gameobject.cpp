#include "common.h"

#include "gameobject.h"
#include "block.h"
#include "roommap.h"
#include "shader.h"
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

void GameObject::shift_pos(Point d, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto self_unique = room_map->take_quiet(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_, room_map));
    }
    pos_.x += d.x;
    pos_.y += d.y;
    room_map->put_quiet(std::move(self_unique));
}

void GameObject::set_pos(Point p, RoomMap* room_map, DeltaFrame* delta_frame) {
    auto self_unique = room_map->take_quiet(this);
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, pos_, room_map));
    }
    pos_.x = p.x;
    pos_.y = p.y;
    room_map->put_quiet(std::move(self_unique));
}

// It is in fact very rare for us to want to move an object without taking and putting,
// as in the ordinary set_pos().  However, moving through doors is such an occasion.
void GameObject::set_pos_raw(Point p) {
    pos_.x = p.x;
    pos_.y = p.y;
}

void GameObject::reinit() {}

void GameObject::cleanup(DeltaFrame* delta_frame) {}

bool GameObject::wall () {
    return false;
}

Wall::Wall(int x, int y): GameObject(x, y) {}

Wall::~Wall() {}

ObjCode Wall::obj_code() {
    return ObjCode::Wall;
}

Layer Wall::layer() {
    return Layer::Solid;
}

bool Wall::wall() {
    return true;
}

void Wall::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f, p.y));
    shader->setMat4("model", model);
    shader->setVec4("color", BLACK);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
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

Block* Player::get_car(RoomMap* room_map) {
    if (state_ != RidingState::Riding) {
        return nullptr;
    } else {
        return static_cast<Block*>(room_map->view(pos_, Layer::Solid));
    }
}

void Player::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 1.0f, p.y));
    model = glm::scale(model, glm::vec3(0.5f, 0.3f, 0.5f));
    shader->setMat4("model", model);
    shader->setVec4("color", PINK);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void Player::serialize(std::ofstream& file) {
    file << static_cast<unsigned char>(state_);
}

GameObject* Player::deserialize(unsigned char* b) {
    return new Player(b[0], b[1], static_cast<RidingState>(b[2]));
}
