#include "gameobject.h"
#include "shader.h"
#include "delta.h"

const glm::vec4 GREEN = glm::vec4(0.6f, 0.9f, 0.7f, 1.0f);
const glm::vec4 PINK = glm::vec4(0.9f, 0.6f, 0.7f, 1.0f);
const glm::vec4 RED = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
const glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 ORANGE = glm::vec4(1.0f, 0.7f, 0.3f, 1.0f);

PosIdMap GameObject::EMPTY_POS_ID_MAP {};

unsigned int GameObject::GLOBAL_ID_COUNT = 0;

unsigned int GameObject::gen_id() {
    return ++GLOBAL_ID_COUNT;
}

GameObject::GameObject(int x, int y): id_ {GameObject::gen_id()}, pos_ {x, y}, wall_ {true} {}

GameObject::~GameObject() {}

unsigned int GameObject::id() const {
    return id_;
}

Layer GameObject::layer() const {
    return Layer::Solid;
}

Point GameObject::pos() const {
    return pos_;
}

bool GameObject::wall() const {
    return wall_;
}


Wall::Wall(int x, int y): GameObject(x, y) {}

Wall::~Wall() {}

void Wall::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - BOARD_SIZE/2, 0.5f, p.y - BOARD_SIZE/2));
    shader->setMat4("model", model);
    shader->setVec4("color", BLACK);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

// Push is the "default" BlockType
Block::Block(int x, int y): GameObject(x, y), type_ {BlockType::Push}, car_ {false} {
    wall_ = false;
}

Block::~Block() {}

void Block::set_car(bool car) {
    car_ = car;
}

void Block::draw(Shader* shader) {
    if (car_) {
        Point p = pos();
        glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - BOARD_SIZE/2, 1.0f, p.y - BOARD_SIZE/2));
        model = glm::scale(model, glm::vec3(0.5f, 0.2f, 0.5f));
        shader->setMat4("model", model);
        shader->setVec4("color", PINK);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
}

BlockType Block::type() {
    return type_;
}

void Block::shift_pos(Point d, DeltaFrame* delta_frame) {
    pos_.x += d.x;
    pos_.y += d.y;
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, d));
    }
}

PushBlock::PushBlock(int x, int y): Block(x, y), sticky_ {StickyLevel::None}, links_ {} {}
PushBlock::PushBlock(int x, int y, StickyLevel sticky): Block(x, y), sticky_ {sticky}, links_ {} {}

PushBlock::~PushBlock() {}

void PushBlock::set_sticky(StickyLevel sticky) {
    sticky_ = sticky;
}

void PushBlock::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - BOARD_SIZE/2, 0.5f, p.y - BOARD_SIZE/2));
    shader->setMat4("model", model);
    if (sticky_ == StickyLevel::None) {
        shader->setVec4("color", GREEN);
    } else if (sticky_ == StickyLevel::Strong) {
        shader->setVec4("color", ORANGE);
    } else /* sticky_ == StickyLevel::Weak */ {
        shader->setVec4("color", RED);
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    Block::draw(shader);
}

StickyLevel PushBlock::sticky() {
    return sticky_;
}

PosIdMap& PushBlock::get_strong_links() {
    if (sticky_ == StickyLevel::Strong) {
        return links_;
    } else {
        return EMPTY_POS_ID_MAP;
    }
}

PosIdMap& PushBlock::get_weak_links() {
    if (sticky_ == StickyLevel::Weak) {
        return links_;
    } else {
        return EMPTY_POS_ID_MAP;
    }
}

void PushBlock::insert_link(PosId p) {
    links_.insert(p);
}
