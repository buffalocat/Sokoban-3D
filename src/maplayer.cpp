#include "maplayer.h"

#include "delta.h"
#include "gameobject.h"
#include "mapfile.h"
#include "graphicsmanager.h"
#include "gameobjectarray.h"


bool MapRect::contains(Point2 p) {
    return (x <= p.x) && (p.x < x + w) && (y <= p.y) && (p.y < y + h);
}


MapLayer::MapLayer(RoomMap* room_map, int z): parent_map_ {room_map}, z_ {z} {}

MapLayer::~MapLayer() {}

/*
template <typename IDFunc>
MapLayer::apply_to_rect(MapRect rect, IDFunc f) {
    switch (type()) {
    case MapCode::FullLayer:
        static_cast<FullMapLayer>(this)->apply_to_rect(rect, f);
    case MapCode::SparseLayer:
        static_cast<SparseMapLayer>(this)->apply_to_rect(rect, f);
    }
}
*/

FullMapLayer::FullMapLayer(RoomMap* room_map, int width, int height, int z):
        MapLayer(room_map, z), map_ {}, width_ {width}, height_ {height} {
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

SparseMapLayer::SparseMapLayer(RoomMap* room_map, int z): MapLayer(room_map, z), map_ {} {}

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
