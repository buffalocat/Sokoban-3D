#include "worldmap.h"
#include <iostream>
#include <unordered_map>

#include "gameobject.h"
#include "delta.h"
#include "block.h"


WorldMap::WorldMap(int width, int height): width_ {width}, height_ {height}, map_ {}, movers_ {} {
    for (int i = 0; i != width; ++i) {
        map_.push_back({});
        for (int j = 0; j != height; ++j) {
            map_[i].push_back(std::move(MapCell()));
        }
    }
}

bool WorldMap::valid(Point pos) {
    return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_);
}

int WorldMap::width() const {
    return width_;
}

int WorldMap::height() const {
    return height_;
}

const std::deque<Block*>& WorldMap::movers() {
    return movers_;
}

// Return a mover, and also cycle camera targets!
Block* WorldMap::prime_mover() {
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
void WorldMap::serialize(std::ofstream& file) const {
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            for (auto& layer : map_[x][y]) {
                for (auto& object : layer) {
                    file << static_cast<unsigned char>(object->obj_code());
                    file << static_cast<unsigned char>(x);
                    file << static_cast<unsigned char>(y);
                    object->serialize(file);
                }
            }
        }
    }
    file << static_cast<unsigned char>(ObjCode::NONE);
}

GameObject* WorldMap::view(Point pos, Layer layer) {
    if (valid(pos)) {
        auto &vec = map_[pos.x][pos.y][static_cast<unsigned int>(layer)];
        if (!vec.empty()) {
            return vec.back().get();
        }
    }
    return nullptr;
}

void WorldMap::take(Point pos, Layer layer, DeltaFrame* delta_frame) {
    if (valid(pos)) {
        auto &vec = map_[pos.x][pos.y][static_cast<unsigned int>(layer)];
        if (!vec.empty()) {
            vec.back()->cleanup(delta_frame);
            delta_frame->push(std::make_unique<DeletionDelta>(std::move(vec.back())));
            vec.pop_back();
        }
    }
}

void WorldMap::take_id(Point pos, Layer layer, GameObject* id, DeltaFrame* delta_frame) {
    if (valid(pos)) {
        auto &vec = map_[pos.x][pos.y][static_cast<unsigned int>(layer)];
        for (auto&& it = vec.begin(); it != vec.end(); ++it) {
            if ((*it).get() == id) {
                (*it)->cleanup(delta_frame);
                delta_frame->push(std::make_unique<DeletionDelta>(std::move(*it)));
                vec.erase(it);
                return;
            }
        }
        throw std::runtime_error("Tried to take an object that wasn't there!");
    }
    throw std::runtime_error("Tried to take an object from an invalid location!");
}

std::unique_ptr<GameObject> WorldMap::take_quiet(Point pos, Layer layer) {
    if (valid(pos)) {
        auto &vec = map_[pos.x][pos.y][static_cast<unsigned int>(layer)];
        if (!vec.empty()) {
            auto obj = std::move(vec.back());
            vec.pop_back();
            return obj;
        }
    }
    return nullptr;
}

std::unique_ptr<GameObject> WorldMap::take_quiet_id(Point pos, Layer layer, GameObject* id) {
    if (valid(pos)) {
        auto &vec = map_[pos.x][pos.y][static_cast<unsigned int>(layer)];
        for (auto&& it = vec.begin(); it != vec.end(); ++it) {
            if ((*it).get() == id) {
                auto obj = std::move(*it);
                vec.erase(it);
                return obj;
            }
        }
        throw std::runtime_error("Tried to (quietly) take an object that wasn't there!");
    }
    throw std::runtime_error("Tried to (quietly) take an object from an invalid location!");
}

void WorldMap::put(std::unique_ptr<GameObject> object, DeltaFrame* delta_frame) {
    Point pos = object->pos();
    if (valid(pos)) {
        delta_frame->push(std::make_unique<CreationDelta>(object.get()));
        map_[pos.x][pos.y][static_cast<unsigned int>(object->layer())].push_back(std::move(object));
    } else {
        throw std::runtime_error("Tried to put an object in an invalid location!");
    }
}

void WorldMap::put_quiet(std::unique_ptr<GameObject> object) {
    Point pos = object->pos();
    if (valid(pos)) {
        Block* block = dynamic_cast<Block*>(object.get());
        map_[pos.x][pos.y][static_cast<unsigned int>(object->layer())].push_back(std::move(object));
    } else {
        throw std::runtime_error("Tried to (quietly) put an object in an invalid location!");
    }
}

void WorldMap::add_mover(Block* block) {
    if (block->car()) {
        movers_.push_back(block);
    }
}

void WorldMap::draw(Shader* shader) {
    for (auto& column : map_) {
        for (auto& cell : column) {
            for (auto& layer : cell) {
                int i = 0;
                for (auto& object : layer) {
                    object->draw(shader, i++);
                }
            }
        }
    }
}

void WorldMap::set_initial_state() {
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
void WorldMap::print_snake_info() {
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
