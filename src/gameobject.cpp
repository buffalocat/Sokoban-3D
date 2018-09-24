#include "common.h"

#include "gameobject.h"
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

// Wall serializes trivially
GameObject* Wall::deserialize(unsigned char* b) {
    return new Wall(b[0], b[1]);
}
