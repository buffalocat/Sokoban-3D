#include "common.h"

#include "gameobject.h"
#include "roommap.h"
#include "shader.h"
#include "delta.h"

GameObject::GameObject(int x, int y): pos_ {x, y}, wall_ {true} {}

GameObject::~GameObject() {}

ObjCode GameObject::obj_code() {
    return code_map[typeid(this)];
}

Layer GameObject::layer() const {
    return Layer::Solid;
}

Point GameObject::pos() const {
    return pos_;
}

Point GameObject::shifted_pos(Point d) const {
    return Point{pos_.x + d.x, pos_.y + d.y};
}

bool GameObject::wall() const {
    return wall_;
}

Wall::Wall(int x, int y): GameObject(x, y) {}

Wall::~Wall() {}

void Wall::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, 0.5f, p.y));
    shader->setMat4("model", model);
    shader->setVec4("color", BLACK);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void Wall::cleanup(DeltaFrame* delta_frame) {}

void Wall::reinit() {}

void Wall::serialize(std::ofstream& file) {}

GameObject* Wall::deserialize(unsigned char* b) {
    return new Wall(b[0], b[1]);
}
