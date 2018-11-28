#include "maplayer.h"

#include "delta.h"
#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"


MapLayer::MapLayer(RoomMap* room_map): parent_map_ {room_map} {}

MapLayer::~MapLayer() {}


FullMapLayer::FullMapLayer(RoomMap* room_map, int width, int height): MapLayer(room_map), map_ {} {
    for (int i = 0; i != width; ++i) {
        map_.push_back({});
        for (int j = 0; j != height; ++j) {
            map_[i].push_back({});
        }
    }
}

FullMapLayer::~FullMapLayer() {}

GameObject* FullMapLayer::view(Point pos) {
    return map_[pos.x][pos.y].get();
}

void FullMapLayer::take(Point pos, DeltaFrame* delta_frame) {
    delta_frame->push(std::make_unique<DeletionDelta>(std::move(map_[pos.x][pos.y]), parent_map_));
}

std::unique_ptr<GameObject> FullMapLayer::take_quiet(Point pos) {
    return std::move(map_[pos.x][pos.y]);
}

void FullMapLayer::put(std::unique_ptr<GameObject> obj, DeltaFrame* delta_frame) {
    Point3 pos {obj->pos()};
    delta_frame->push(std::make_unique<CreationDelta>(pos, parent_map_));
    map_[pos.x][pos.y] = std::move(obj);
}

void FullMapLayer::put_quiet(std::unique_ptr<GameObject> obj) {
    Point3 pos {obj->pos()};
    map_[pos.x][pos.y] = std::move(obj);
}

MapCode FullMapLayer::type() {
    return MapCode::FullLayer;
}

void FullMapLayer::serialize(MapFileO& file, std::vector<GameObject*>& rel_check) {
    for (auto& col : map_) {
        for (auto& obj : col) {
            if (!obj || obj->obj_code() == ObjCode::Player) {
                continue;
            }
            file << obj->obj_code();
            file << obj->pos();
            obj->serialize(file);
            if (obj->relation_check()) {
                rel_check.push_back(obj.get());
            }
        }
    }
}

void FullMapLayer::draw(GraphicsManager* gfx) {
    for (auto& col : map_) {
        for (auto& obj : col) {
            if (obj) {
                obj->draw(gfx);
            }
        }
    }
}


SparseMapLayer::SparseMapLayer(RoomMap* room_map): MapLayer(room_map), map_ {} {}

SparseMapLayer::~SparseMapLayer() {}

GameObject* SparseMapLayer::view(Point pos) {
    return map_.count(pos) ? map_.at(pos).get() : nullptr;
}

void SparseMapLayer::take(Point pos, DeltaFrame* delta_frame) {
    auto obj = std::move(map_[pos]);
    map_.erase(pos);
    delta_frame->push(std::make_unique<DeletionDelta>(std::move(obj), parent_map_));
}

std::unique_ptr<GameObject> SparseMapLayer::take_quiet(Point pos) {
    auto obj = std::move(map_[pos]);
    map_.erase(pos);
    return obj;
}

void SparseMapLayer::put(std::unique_ptr<GameObject> obj, DeltaFrame* delta_frame) {
    delta_frame->push(std::make_unique<CreationDelta>(obj->pos(), parent_map_));
    map_[obj->posh()] = std::move(obj);
}

void SparseMapLayer::put_quiet(std::unique_ptr<GameObject> obj) {
    map_[obj->posh()] = std::move(obj);
}

MapCode SparseMapLayer::type() {
    return MapCode::SparseLayer;
}

void SparseMapLayer::serialize(MapFileO& file, std::vector<GameObject*>& rel_check) {
    for (auto& p : map_) {
        GameObject* obj = p.second.get();
        if (!obj || obj->obj_code() == ObjCode::Player) {
            continue;
        }
        file << obj->obj_code();
        file << obj->pos();
        obj->serialize(file);
        if (obj->relation_check()) {
            rel_check.push_back(obj);
        }
    }
}

void SparseMapLayer::draw(GraphicsManager* gfx) {
    for (auto& p : map_) {
        GameObject* obj = p.second.get();
        if (obj) {
            obj->draw(gfx);
        }
    }
}
