#include "worldmap.h"

Point negate(Point p) {
    return Point {.x = -p.x, .y = -p.y};
}

GameObject::GameObject(int x, int y) {
    pos_ = {.x = x, .y = y};

}

unsigned int GameObject::id() const {
    return id_;
}

Layer GameObject::layer() const {
    return layer_;
}

Point GameObject::pos() const {
    return pos_;
}

void GameObject::shift_pos(Point d, DeltaFrame* delta_frame) {
    pos_.x += d.x;
    pos_.y += d.y;
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(*this, d));
    }
}

DeltaFrame::DeltaFrame() {
    std::vector<std::unique_ptr<Delta>> deltas_();
}

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

DeletionDelta::DeletionDelta(std::unique_ptr<GameObject> object) {
    object_ = std::move(object);
}

void DeletionDelta::revert(WorldMap* world_map) {
    world_map->put_quiet(std::move(object_));
}

CreationDelta::CreationDelta(GameObject const& object) {
    pos_ = object.pos();
    layer_ = object.layer();
    id_ = object.id();
}

void CreationDelta::revert(WorldMap* world_map) {
    world_map->take_quiet(pos_, layer_, id_);
}

MotionDelta::MotionDelta(GameObject const& object, Point d) {
    pos_ = object.pos();
    layer_ = object.layer();
    id_ = object.id();
    d_ = d;
}

void MotionDelta::revert(WorldMap* world_map) {
    auto object = world_map->take_quiet(pos_, layer_, id_);
    // We don't want to create any deltas while undoing a delta (yet, at least)
    object->shift_pos(negate(d_), NULL);
    world_map->put_quiet(std::move(object));
}

MapCell::MapCell() {
    std::array<std::vector<std::unique_ptr<GameObject>>, static_cast<unsigned int>(Layer::COUNT)> layers_;
}

// NULL means this Layer of this Cell was empty (this may happen often)
GameObject const* MapCell::view(Layer layer) {
    if (layers_[static_cast<unsigned int>(layer)].empty()) {
        return NULL;
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

WorldMap::WorldMap(int width, int height) {
    width_ = width;
    height_ = height;
    std::vector<std::vector<MapCell>> map_;
    for (int i = 0; i < width; ++i) {
        map_.push_back(std::vector<MapCell>());
        for (int j = 0; j < height; ++j) {
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
        return NULL;
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
    map_[pos.x][pos.y].put(std::move(object), delta_frame);
}

void WorldMap::put_quiet(std::unique_ptr<GameObject> object) {
    auto pos = object->pos();
    map_[pos.x][pos.y].put_quiet(std::move(object));
}
