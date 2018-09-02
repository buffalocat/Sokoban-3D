#include "worldmap.h"
#include <iostream>

const glm::vec4 GREEN = glm::vec4(0.6f, 0.9f, 0.7f, 1.0f);
const glm::vec4 PINK = glm::vec4(0.9f, 0.6f, 0.7f, 1.0f);
const glm::vec4 BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

unsigned int GameObject::GLOBAL_ID_COUNT = 0;

unsigned int GameObject::gen_id() {
    return GLOBAL_ID_COUNT++;
}

GameObject::GameObject(int x, int y): id_ {GameObject::gen_id()}, pos_ {x, y} {}

GameObject::~GameObject() {}

unsigned int GameObject::id() const {
    return id_;
}

Point GameObject::pos() const {
    return pos_;
}

Layer GameObject::layer() const {
    return Layer::Solid;
}

void GameObject::shift_pos(Point d, DeltaFrame* delta_frame) {
    pos_.x += d.x;
    pos_.y += d.y;
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(*this, d));
    }
}

Car::Car(int x, int y): GameObject(x, y) {}

Car::~Car() {}

bool Car::pushable() const { return true; }

Layer Car::layer() const { return Layer::Solid; }

void Car::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - 5.0f, 0.5f, p.y - 5.0f));
    shader->setMat4("model", model);
    shader->setVec4("color", PINK);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}


Block::Block(int x, int y): GameObject(x, y) {}

Block::~Block() {}

bool Block::pushable() const { return true; }

Layer Block::layer() const { return Layer::Solid; }

void Block::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - 5.0f, 0.5f, p.y - 5.0f));
    shader->setMat4("model", model);
    shader->setVec4("color", GREEN);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}


Wall::Wall(int x, int y): GameObject(x, y) {}

Wall::~Wall() {}

bool Wall::pushable() const { return false; }

Layer Wall::layer() const { return Layer::Solid; }

void Wall::draw(Shader* shader) {
    Point p = pos();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x - 5.0f, 0.5f, p.y - 5.0f));
    shader->setMat4("model", model);
    shader->setVec4("color", BLACK);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}


DeltaFrame::DeltaFrame(): deltas_ {} {}

void DeltaFrame::revert(WorldMap* world_map) {
    for (auto& delta : deltas_) {
        (*delta).revert(world_map);
    }
}

void DeltaFrame::push(std::unique_ptr<Delta> delta) {
    deltas_.push_back(std::move(delta));
}

bool DeltaFrame::trivial() {
    return deltas_.empty();
}

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object): object_ {std::move(object)} {}

void DeletionDelta::revert(WorldMap* world_map) {
    world_map->put_quiet(std::move(object_));
}

CreationDelta::CreationDelta(GameObject const& object): pos_ {object.pos()}, layer_ {object.layer()}, id_ {object.id()} {}

void CreationDelta::revert(WorldMap* world_map) {
    world_map->take_quiet(pos_, layer_, id_);
}

MotionDelta::MotionDelta(GameObject const& object, Point d): pos_ {object.pos()}, layer_ {object.layer()}, id_ {object.id()}, d_ {d} {}

void MotionDelta::revert(WorldMap* world_map) {
    auto object = world_map->take_quiet(pos_, layer_, id_);
    // We don't want to create any deltas while undoing a delta (yet, at least)
    object->shift_pos(Point{-d_.x, -d_.y}, nullptr);
    world_map->put_quiet(std::move(object));
}

MapCell::MapCell(): layers_(std::array<std::vector<std::unique_ptr<GameObject>>, static_cast<unsigned int>(Layer::COUNT)>()) {}

// NULL means this Layer of this Cell was empty (this may happen often)
GameObject const* MapCell::view(Layer layer) {
    if (layers_[static_cast<unsigned int>(layer)].empty()) {
        return nullptr;
    } else {
        return layers_[static_cast<unsigned int>(layer)].front().get();
    }
}

// If we reach the throw, the object with the given id wasn't where we thought it was
// (and this is probably a sign of a bug, so we'll throw an exception for now)
GameObject const* MapCell::view_id(Layer layer, unsigned int id) {
    for (auto const& object : layers_[static_cast<unsigned int>(layer)]) {
        if (object.get()->id() == id) {
            return object.get();
        }
    }
    throw "Object not found in call to view_id!";
}

void MapCell::take(Layer layer, unsigned int id, DeltaFrame* delta_frame) {
    auto &vec = layers_[static_cast<unsigned int>(layer)];
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if ((*it).get()->id() == id) {
            delta_frame->push(static_cast<std::unique_ptr<Delta>>(std::make_unique<DeletionDelta>(std::move(*it))));
            vec.erase(it);
            return;
        }
    }
    throw "Object not found in call to take!";
}

std::unique_ptr<GameObject> MapCell::take_quiet(Layer layer, unsigned int id) {
    auto &vec = layers_[static_cast<unsigned int>(layer)];
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if ((*it).get()->id() == id) {
            auto obj = std::move(*it);
            vec.erase(it);
            return obj;
        }
    }
    throw "Object not found in call to take_quiet!";
}

void MapCell::put(std::unique_ptr<GameObject> object, DeltaFrame* delta_frame) {
    delta_frame->push(std::make_unique<CreationDelta>(*object));
    layers_[static_cast<unsigned int>(object->layer())].push_back(std::move(object));
}

void MapCell::put_quiet(std::unique_ptr<GameObject> object) {
    layers_[static_cast<unsigned int>(object->layer())].push_back(std::move(object));
}

void MapCell::draw(Shader* shader) {
    for (auto& layer : layers_) {
        for (auto& object : layer) {
            object->draw(shader);
        }
    }
}

WorldMap::WorldMap(int width, int height): width_ {width}, height_ {height}, map_ {} {
    for (int i = 0; i != width; ++i) {
        map_.push_back({});
        for (int j = 0; j != height; ++j) {
            map_[i].push_back(MapCell());
        }
    }
}

bool WorldMap::valid(Point pos) {
    return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_);
}

GameObject const* WorldMap::view(Point pos, Layer layer) {
    if (valid(pos)) {
        return map_[pos.x][pos.y].view(layer);
    } else {
        // This is acceptable: we should already be considering the case
        // that there wasn't an object at the location we checked.
        return nullptr;
    }
}

GameObject const* WorldMap::view_id(Point pos, Layer layer, unsigned int id) {
    if (valid(pos)) {
        return map_[pos.x][pos.y].view_id(layer, id);
    } else {
        throw "Tried to view a (specific!!) object from an invalid location!";
    }
}

void WorldMap::take(Point pos, Layer layer, unsigned int id, DeltaFrame* delta_frame) {
    if (valid(pos)) {
        map_[pos.x][pos.y].take(layer, id, delta_frame);
    } else {
        throw "Tried to take an object from an invalid location!";
    }
}

std::unique_ptr<GameObject> WorldMap::take_quiet(Point pos, Layer layer, unsigned int id) {
    if (valid(pos)) {
        return map_[pos.x][pos.y].take_quiet(layer, id);
    } else {
        throw "Tried to (quietly) take an object from an invalid location!";
    }
}

void WorldMap::put(std::unique_ptr<GameObject> object, DeltaFrame* delta_frame) {
    auto pos = object->pos();
    if (valid(pos)) {
        map_[pos.x][pos.y].put(std::move(object), delta_frame);
    } else {
        throw "Tried to put an object in an invalid location!";
    }
}

void WorldMap::put_quiet(std::unique_ptr<GameObject> object) {
    auto pos = object->pos();
    if (valid(pos)) {
        map_[pos.x][pos.y].put_quiet(std::move(object));
    } else {
        throw "Tried to (quietly) put an object in an invalid location!";
    }
}

void WorldMap::move_player(GameObject* player, Point dir) {
    Point cur_pos = player->pos();
    Point new_pos {cur_pos.x + dir.x, cur_pos.y + dir.y};
    if (valid(new_pos) && !view(new_pos, player->layer())) {
        auto object = take_quiet(cur_pos, player->layer(), player->id());
        // We'll implement deltas soon, but not yet
        object->shift_pos(dir, nullptr);
        put_quiet(std::move(object));
    }
}

void WorldMap::draw(Shader* shader) {
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            map_[i][j].draw(shader);
        }
    }
}
