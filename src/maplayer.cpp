#include "maplayer.h"

#include "delta.h"
#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "gameobjectarray.h"


bool MapRect::contains(Point2 p) {
    return (x <= p.x) && (p.x < x + w) && (y <= p.y) && (p.y < y + h);
}


MapLayer::MapLayer(RoomMap* room_map): parent_map_ {room_map} {}

MapLayer::~MapLayer() {}


FullMapLayer::FullMapLayer(RoomMap* room_map, int width, int height):
        MapLayer(room_map), map_ {}, width_ {width}, height_ {height} {
    for (int i = 0; i != width; ++i) {
        map_.push_back(std::vector<int>(height, 0));
    }
}

FullMapLayer::~FullMapLayer() {}

int& FullMapLayer::at(Point2 pos) {
    return map_[pos.x][pos.y];
}

MapCode FullMapLayer::type() {
    return MapCode::FullLayer;
}

void FullMapLayer::apply_to_rect(MapRect rect, GameObjIDFunc& f) {
    for (int i = rect.x; i < rect.x + rect.w; ++i) {
        for (int j = rect.y; j < rect.y + rect.h; ++j) {
            if (int id = map_[i][j]) {
                f(id);
            }
        }
    }
}

void FullMapLayer::shift_by(int dx, int dy) {
    width_ += dx;
    height_ += dy;
    if (dy < 0) {
        for (auto& row : map_) {
            row.erase(row.begin(), row.begin() - dy);
        }
    } else if (dy > 0) {
        for (auto& row : map_) {
            row.insert(row.begin(), dy, 0);
        }
    }
    if (dx < 0) {
        map_.erase(map_.begin(), map_.begin() - dx);
    } else {
        map_.insert(map_.begin(), dx, std::vector<int>(height_,0));
    }
}

void FullMapLayer::extend_by(int dx, int dy) {
    width_ += dx;
    height_ += dy;
    if (dy < 0) {
        for (auto& row : map_) {
            row.erase(row.end() + dy, row.end());
        }
    } else if (dy > 0) {
        for (auto& row : map_) {
            row.insert(row.end(), dy, 0);
        }
    }
    if (dx < 0) {
        map_.erase(map_.end() + dx, map_.end());
    } else {
        map_.insert(map_.end(), dx, std::vector<int>(height_,0));
    }
}


SparseMapLayer::SparseMapLayer(RoomMap* room_map): MapLayer(room_map), map_ {} {}

SparseMapLayer::~SparseMapLayer() {}

// TODO: fix the way that SparseMapLayers can fill up with empty data
int& SparseMapLayer::at(Point2 pos) {
    return map_[pos];
}

MapCode SparseMapLayer::type() {
    return MapCode::SparseLayer;
}

void SparseMapLayer::apply_to_rect(MapRect rect, GameObjIDFunc& f) {
    for (auto& p : map_) {
        if (rect.contains(p.first) && p.second) {
            f(p.second);
        }
    }
}

void SparseMapLayer::shift_by(int dx, int dy) {
    Point2 dpos = {dx,dy};
    std::unordered_map<Point2, int, Point2Hash> new_map {};
    for (auto& p : map_) {
        new_map[p.first + dpos] = p.second;
    }
    map_ = std::move(new_map);
}

void SparseMapLayer::extend_by(int dx, int dy) {}
