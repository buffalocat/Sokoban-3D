#include "roommap.h"
#include <iostream>
#include <unordered_map>

#include "gameobject.h"
#include "delta.h"
#include "block.h"


RoomMap::RoomMap(int width, int height): width_ {width}, height_ {height}, map_ {}, movers_ {} {
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

const std::deque<Block*>& RoomMap::movers() {
    return movers_;
}

Block* RoomMap::get_mover() {
    if (!movers_.empty()) {
        return movers_.back();
    } else {
        return nullptr;
    }
}

// Return a mover, and also cycle camera targets!
Block* RoomMap::cycle_movers() {
    if (!movers_.empty()) {
        Block* cur = movers_.back();
        movers_.pop_back();
        movers_.push_front(cur);
        return cur;
    } else {
        return nullptr;
    }
}

// Assuming not big_map
void RoomMap::serialize(std::ofstream& file) const {
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            for (auto& object : map_[x][y]) {
                file << static_cast<unsigned char>(object->obj_code());
                file << static_cast<unsigned char>(x);
                file << static_cast<unsigned char>(y);
                object->serialize(file);
            }
        }
    }
    file << static_cast<unsigned char>(ObjCode::NONE);
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

void RoomMap::take(GameObject* object, DeltaFrame* delta_frame) {
    Point pos = object->pos();
    auto &vec = map_[pos.x][pos.y];
    for (auto&& it = vec.begin(); it != vec.end(); ++it) {
        if ((*it).get() == object) {
            (*it)->cleanup(delta_frame);
            delta_frame->push(std::make_unique<DeletionDelta>(std::move(*it)));
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
        delta_frame->push(std::make_unique<CreationDelta>(object.get()));
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

void RoomMap::add_mover(Block* block) {
    if (block->car()) {
        movers_.push_back(block);
    }
}

void RoomMap::draw(Shader* shader) {
    for (auto& column : map_) {
        for (auto& cell : column) {
            for (auto& object : cell) {
                object->draw(shader);
            }
        }
    }
}

void RoomMap::set_initial_state() {
    std::unordered_set<SnakeBlock*> available_snakes = {};
    movers_ = {};
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            auto block = dynamic_cast<Block*>(view(Point{x,y}, Layer::Solid));
            if (block) {
                add_mover(block);
                block->check_add_local_links(this, nullptr);

                auto sb = dynamic_cast<SnakeBlock*>(block);
                if (sb && sb->available() && !sb->confused(this)) {
                    available_snakes.insert(sb);
                }
            }
        }
    }
    // Add links for all snakes!
    for (auto sb : available_snakes) {
        for (auto& d : DIRECTIONS) {
            auto adj = dynamic_cast<SnakeBlock*>(view(sb->shifted_pos(d), Layer::Solid));
            if (adj && available_snakes.count(adj)) {
                sb->add_link(adj, nullptr);
            }
        }
    }
}

// NOTE: Just for debugging! But snakes still have problems, so we still need it.
void RoomMap::print_snake_info() {
    std::cout << std::endl;
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            auto sb = dynamic_cast<SnakeBlock*>(view(Point{x,y}, Layer::Solid));
            if (sb) {
                std::cout << "SnakeBlock at " << sb->pos() <<
                    " with dist " << sb->distance() << //" and target " << sb->target() <<
                    " and it's avaiable? " << sb->available() << " and confused? " << sb->confused(this) << std::endl;
            }
        }
    }
}
