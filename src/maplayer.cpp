#include "maplayer.h"

#include "delta.h"
#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"


MapLayerIterator::~MapLayerIterator() {}

// TODO: Make the Iterator more like a real Iterator!!!!!!!
FullMapLayerIterator::FullMapLayerIterator(std::vector<std::vector<int>>& map, int z, int width, int height):
MapLayerIterator(),
map_ {map}, pos_ {0, 0, z}, width_ {width}, height_ {height} {}

FullMapLayerIterator::~FullMapLayerIterator() {}

void FullMapLayerIterator::advance() {
    ++pos_.x;
    if (pos_.x == width_) {
        pos_.x = 0;
        ++pos_.y;
    }
}

bool FullMapLayerIterator::done() {
    return pos_.y == height_;
}

Point3 FullMapLayerIterator::pos() {
    return pos_;
}

int FullMapLayerIterator::id() {
    return map_[pos_.x][pos_.y];
}


SparseMapLayerIterator::SparseMapLayerIterator(std::unordered_map<Point2, int, Point2Hash>& map, int z):
MapLayerIterator(),
iter_ {map.begin()}, end_ {map.end()}, pos_ {0, 0, z}, id_ {-1} {
    update_values();
}

SparseMapLayerIterator::~SparseMapLayerIterator() {}

void SparseMapLayerIterator::advance() {
    ++iter_;
    update_values();
}

bool SparseMapLayerIterator::done() {
    return iter_ != end_;
}

void SparseMapLayerIterator::update_values() {
    if (!done()) {
        auto p = *iter_;
        pos_.x = p.first.x;
        pos_.y = p.first.y;
        id_ = p.second;
    }
}

Point3 SparseMapLayerIterator::pos() {
    return pos_;
}

int SparseMapLayerIterator::id() {
    return id_;
}


MapLayer::MapLayer(RoomMap* room_map, int z): parent_map_ {room_map}, z_ {z} {}

MapLayer::~MapLayer() {}

FullMapLayer::FullMapLayer(RoomMap* room_map, int width, int height, int z): MapLayer(room_map, z), map_ {} {
    for (int i = 0; i != width; ++i) {
        map_.push_back({});
        for (int j = 0; j != height; ++j) {
            map_[i].push_back({});
        }
    }
}

FullMapLayer::~FullMapLayer() {}

int& FullMapLayer::at(Point2 pos) {
    return map_[pos.x][pos.y];
}

MapCode FullMapLayer::type() {
    return MapCode::FullLayer;
}

std::unique_ptr<MapLayerIterator> FullMapLayer::begin_iter() {
    return std::make_unique<FullMapLayerIterator>(map_, z_, width_, height_);
}


SparseMapLayer::SparseMapLayer(RoomMap* room_map, int z): MapLayer(room_map, z), map_ {} {}

SparseMapLayer::~SparseMapLayer() {}

// TODO: fix the way that SparseMapLayers can fill up with empty data
int& SparseMapLayer::at(Point2 pos) {
    return map_[pos];
}

MapCode SparseMapLayer::type() {
    return MapCode::SparseLayer;
}

std::unique_ptr<MapLayerIterator> SparseMapLayer::begin_iter() {
    return std::make_unique<SparseMapLayerIterator>(map_, z_);
}
