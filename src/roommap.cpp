#include "roommap.h"
#include <iostream>
#include <unordered_map>

#include "gameobject.h"
#include "delta.h"
#include "block.h"
#include "switch.h"

RoomMap::RoomMap(int width, int height): width_ {width}, height_ {height}, map_ {} {
    for (int i = 0; i != width; ++i) {
        map_.push_back({});
        for (int j = 0; j != height; ++j) {
            map_[i].push_back(std::move(MapCell()));
        }
    }
}

bool RoomMap::valid(Point pos) {
    return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_);
}

int RoomMap::width() const {
    return width_;
}

int RoomMap::height() const {
    return height_;
}

void RoomMap::serialize(std::ofstream& file) const {
    std::vector<GameObject*> rel_check;
    // Serialize raw object data
    file << static_cast<unsigned char>(MapCode::Objects);
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            for (auto& object : map_[x][y]) {
                if (object->obj_code() == ObjCode::Player) {
                    continue;
                }
                file << static_cast<unsigned char>(object->obj_code());
                file << static_cast<unsigned char>(x);
                file << static_cast<unsigned char>(y);
                object->serialize(file);
                if (object->relation_check()) {
                    rel_check.push_back(object.get());
                }
            }
        }
    }
    file << static_cast<unsigned char>(ObjCode::NONE);
    // Serialize relational data
    for (auto& object : rel_check) {
        object->relation_serialize(file);
    }
}

GameObject* RoomMap::view(Point pos, Layer layer) {
    if (valid(pos)) {
        for (auto& obj : map_[pos.x][pos.y]) {
            if (obj->layer() == layer) {
                return obj.get();
            }
        }
    }
    return nullptr;
}

GameObject* RoomMap::view(Point pos, ObjCode code) {
    if (valid(pos)) {
        for (auto& obj : map_[pos.x][pos.y]) {
            if (obj->obj_code() == code) {
                return obj.get();
            }
        }
    }
    return nullptr;
}

#define ROOMMAP_VIEW(CLASS)\
CLASS* RoomMap::view_ ## CLASS(Point pos) {\
    if (valid(pos)) {\
        for (auto& obj : map_[pos.x][pos.y]) {\
            CLASS* ptr = dynamic_cast<CLASS*>(obj.get());\
            if (ptr) {\
                return ptr;\
            }\
        }\
    }\
    return nullptr;\
}

ROOMMAP_VIEW(Switchable)
ROOMMAP_VIEW(Switch)

#undef ROOMMAP_VIEW

void RoomMap::take(GameObject* object, DeltaFrame* delta_frame) {
    Point pos = object->pos();
    auto &vec = map_[pos.x][pos.y];
    for (auto&& it = vec.begin(); it != vec.end(); ++it) {
        if ((*it).get() == object) {
            (*it)->cleanup(delta_frame);
            delta_frame->push(std::make_unique<DeletionDelta>(std::move(*it), this));
            vec.erase(it);
            return;
        }
    }
    throw std::runtime_error("Tried to take an object that wasn't there!");
}

std::unique_ptr<GameObject> RoomMap::take_quiet(GameObject* object) {
    Point pos = object->pos();
    auto &vec = map_[pos.x][pos.y];
    for (auto&& it = vec.begin(); it != vec.end(); ++it) {
        if ((*it).get() == object) {
            auto obj = std::move(*it);
            vec.erase(it);
            return obj;
        }
    }
    throw std::runtime_error("Tried to (quietly) take an object that wasn't there!");
}

void RoomMap::put(std::unique_ptr<GameObject> object, DeltaFrame* delta_frame) {
    if (!object) {
        return;
    }
    Point pos = object->pos();
    if (valid(pos)) {
        delta_frame->push(std::make_unique<CreationDelta>(object.get(), this));
        map_[pos.x][pos.y].push_back(std::move(object));
    } else {
        throw std::runtime_error("Tried to put an object in an invalid location!");
    }
}

void RoomMap::put_quiet(std::unique_ptr<GameObject> object) {
    if (!object) {
        return;
    }
    Point pos = object->pos();
    if (valid(pos)) {
        Block* block = dynamic_cast<Block*>(object.get());
        map_[pos.x][pos.y].push_back(std::move(object));
    } else {
        throw std::runtime_error("Tried to (quietly) put an object in an invalid location!");
    }
}

void RoomMap::draw(GraphicsManager* gfx) {
    for (auto& column : map_) {
        for (auto& cell : column) {
            for (auto& object : cell) {
                object->draw(gfx);
            }
        }
    }
}

void RoomMap::set_initial_state(bool editor_mode) {
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            auto pb = dynamic_cast<PushBlock*>(view(Point{x,y}, Layer::Solid));
            if (pb) {
                pb->check_add_local_links(this, nullptr);
            }
            if (editor_mode) {
                continue;
            }
            auto gate = dynamic_cast<Gate*>(view(Point{x,y}, ObjCode::Gate));
            if (gate) {
                gate->check_waiting(this, nullptr);
            }
        }
    }
}

void RoomMap::print_contents() {
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            for (auto& obj : map_[x][y]) {
                std::cout << "Object type " << static_cast<int>(obj->obj_code()) << " at " << x << "," << y << " thinks it's at " << obj->pos() << std::endl;
            }
        }
    }
}
