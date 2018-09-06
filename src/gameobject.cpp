#include "worldmap.h"
#include "gameobject.h"
#include "shader.h"
#include "delta.h"

const glm::vec4 GREEN = glm::vec4(0.6f, 0.9f, 0.7f, 1.0f);
const glm::vec4 PINK = glm::vec4(0.9f, 0.6f, 0.7f, 1.0f);
const glm::vec4 PURPLE = glm::vec4(0.7f, 0.5f, 0.9f, 1.0f);
const glm::vec4 DARK_PURPLE = glm::vec4(0.3f, 0.2f, 0.6f, 1.0f);
const glm::vec4 RED = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
const glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 ORANGE = glm::vec4(1.0f, 0.7f, 0.3f, 1.0f);

ObjSet GameObject::EMPTY_OBJ_SET {};

GameObject::GameObject(int x, int y): pos_ {x, y}, wall_ {true} {}

GameObject::~GameObject() {}

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
Block::Block(int x, int y): GameObject(x, y), car_ {false}, links_ {} {
    wall_ = false;
}

Block::~Block() {}

void Block::set_car(bool car) {
    car_ = car;
}

void Block::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model;
    if (car_) {
        model = glm::translate(glm::mat4(), glm::vec3(p.x - BOARD_SIZE/2, 1.0f, p.y - BOARD_SIZE/2));
        model = glm::scale(model, glm::vec3(0.5f, 0.2f, 0.5f));
        shader->setMat4("model", model);
        shader->setVec4("color", PINK);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
    // Debugging mode!! Maybe this will be toggle-able later?
    shader->setVec4("color", BLACK);
    for (auto link : links_) {
        Point q = link->pos();
        Point d {q.x - p.x, q.y - p.y};
        model = glm::translate(glm::mat4(), glm::vec3(0.2f*d.x, 0.5f, 0.2f*d.y));
        model = glm::translate(model, glm::vec3(p.x - BOARD_SIZE/2, 0.5f, p.y - BOARD_SIZE/2));
        model = glm::scale(model, glm::vec3(0.1f + 0.2f*abs(d.x), 0.1f, 0.1f + 0.2f*abs(d.y)));
        shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    }
}

void Block::shift_pos(Point d, DeltaFrame* delta_frame) {
    pos_.x += d.x;
    pos_.y += d.y;
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(this, d));
    }
}

ObjSet Block::links() {
    return links_;
}

PushBlock::PushBlock(int x, int y): Block(x, y), sticky_ {StickyLevel::None} {}
PushBlock::PushBlock(int x, int y, StickyLevel sticky): Block(x, y), sticky_ {sticky} {}

PushBlock::~PushBlock() {}

void PushBlock::set_sticky(StickyLevel sticky) {
    sticky_ = sticky;
}

void PushBlock::set_links(ObjSet links) {
    links_ = links;
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

const ObjSet& PushBlock::get_strong_links() {
    if (sticky_ == StickyLevel::Strong) {
        return links_;
    } else {
        return EMPTY_OBJ_SET;
    }
}

const ObjSet& PushBlock::get_weak_links() {
    if (sticky_ == StickyLevel::Weak) {
        return links_;
    } else {
        return EMPTY_OBJ_SET;
    }
}

SnakeBlock::SnakeBlock(int x, int y): Block(x, y), ends_ {2} {}
SnakeBlock::SnakeBlock(int x, int y, unsigned int ends): Block(x, y), ends_ {(ends == 1 || ends == 2) ? ends : 2} {}

SnakeBlock::~SnakeBlock() {}

const ObjSet& SnakeBlock::get_strong_links() {
    return EMPTY_OBJ_SET;
}

const ObjSet& SnakeBlock::get_weak_links() {
    return links_;
}

void SnakeBlock::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - BOARD_SIZE/2, 0.5f, p.y - BOARD_SIZE/2));
    model = glm::scale(model, glm::vec3(0.7071f, 1, 0.7071f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
    shader->setMat4("model", model);
    if (ends_ == 1) {
        shader->setVec4("color", PURPLE);
    } else {
        shader->setVec4("color", DARK_PURPLE);
    }
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    Block::draw(shader);
}

/*
void SnakeBlock::set_potential_links(WorldMap* world_map) {
    Point p = pos();
    PosIdMap p_links {};
    int count = 0
    for (int dx : {1,-1}) {
        SnakeBlock* adj = dynamic_cast<SnakeBlock*>(world_map->view(Point{p.x+dx, p.y}, Layer::Solid));
        if (adj) {
            ++count;
            p_links.insert(std::make_pair(Point {dx,0}, adj));
        }
    }
    for (int dy : {1,-1}) {
        SnakeBlock* adj = dynamic_cast<SnakeBlock*>(world_map->view(Point{p.x, p.y+dy}, Layer::Solid));
        if (adj) {
            ++count;
            p_links.insert(std::make_pair(Point {0,dy}, adj));
        }
    }
    if (new_links != links_) {
        if (delta_frame)
            delta_frame->push(std::make_unique<LinkUpdateDelta>(this, links_));
        links_ = new_links;
    }
}*/
