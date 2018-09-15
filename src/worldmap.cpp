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

const std::vector<Block*>& WorldMap::movers() {
    return movers_;
}

Block* WorldMap::prime_mover() {
    if (!movers_.empty()) {
        return movers_.back();
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

/*
// Note: many things are predicated on the assumption that, in any consistent game state,
// certain layers allow at most one object per MapCell.  These include Solid and Player.
void WorldMap::move_solid(Point dir, DeltaFrame* delta_frame) {
    reset_state();
    PosIdMap result {};
    bool move_successful = false;
    // NOTE: This only moves the first thing that can move, for now... i.e., it's "non critically broken"
    for (auto& block : movers_) {
        PosIdMap branch {};
        if (move_strong_component(branch, block->pos(), dir)) {
            move_successful = true;
            result = branch;
            break;
        }
    }
    if (!move_successful) {
        return;
    }
    pull_snakes(delta_frame);
    for (auto& pos_id : result) {
        Point pos;
        GameObject* obj;
        std::tie(pos, obj) = pos_id;
        auto obj_unique = take_quiet_id(pos, Layer::Solid, obj);
        static_cast<Block*>(obj)->shift_pos(dir, delta_frame);
        put_quiet(std::move(obj_unique));
        // Record the old and current positions
        floor_update_.insert(pos);
        floor_update_.insert(obj->pos());
        moved_.insert(obj);
    }
    update_snakes(delta_frame);
    update_links(delta_frame);
}



void WorldMap::set_initial_state() {
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            auto obj = view(Point{x,y}, Layer::Solid);
            update_links_auxiliary(obj, nullptr);
            auto sb = dynamic_cast<SnakeBlock*>(obj);
            if (sb)
                snakes_.insert(sb);
        }
    }
    update_snakes(nullptr);
}

void WorldMap::update_snakes(DeltaFrame* delta_frame) {
    std::unordered_set<SnakeBlock*> available_snakes {};
    for (auto sb : snakes_) {
        if (sb->links().size() == sb->ends()) {
            continue;
        }
        Point pos = sb->pos();
        BlockSet potential_links {};
        for (Point d : {Point{1,0}, Point{-1,0}, Point{0,1}, Point{0,-1}}) {
            auto adj = dynamic_cast<SnakeBlock*>(view(Point{pos.x + d.x, pos.y + d.y}, Layer::Solid));
            if (adj && adj->links().size() < adj->ends()) {
                potential_links.insert(adj);
            }
        }
        if (potential_links.size() <= sb->ends()) {
            available_snakes.insert(sb);
        }
    }
    for (auto sb : available_snakes) {
        Point pos = sb->pos();
        BlockSet links = sb->links();
        for (Point d : {Point{1,0}, Point{-1,0}, Point{0,1}, Point{0,-1}}) {
            auto adj = dynamic_cast<SnakeBlock*>(view(Point{pos.x + d.x, pos.y + d.y}, Layer::Solid));
            if (adj && available_snakes.count(adj)) {
                sb->add_link(adj, delta_frame);
            }
        }
    }
}

void WorldMap::update_links(DeltaFrame* delta_frame) {
    for (auto& obj : moved_) {
        update_links_auxiliary(obj, delta_frame);
    }
    for (auto& obj : link_update_) {
        update_links_auxiliary(obj, delta_frame);
    }
}

void WorldMap::update_links_auxiliary(GameObject* obj, DeltaFrame* delta_frame) {
    auto pb = dynamic_cast<PushBlock*>(obj);
    if (pb && pb->sticky() != StickyLevel::None) {
        Point p = obj->pos();
        BlockSet new_links {};
        for (Point d : {Point{1,0}, Point{-1,0}, Point{0,1}, Point{0,-1}}) {
            PushBlock* adj = dynamic_cast<PushBlock*>(view(Point {p.x + d.x, p.y + d.y}, Layer::Solid));
            if (adj && pb->sticky() == adj->sticky()) {
                pb->add_link(adj, delta_frame);
            }
        }
    }
}


void WorldMap::pull_snakes(DeltaFrame* delta_frame) {
    for (auto pushed : pushed_snakes_) {
        if (not_move_.count(pushed->pos()))
            continue;
        for (auto obj : pushed->links()) {
            SnakeBlock* cur = static_cast<SnakeBlock*>(obj);
            if (not_move_.count(cur->pos())) {
                pushed->remove_link(cur, delta_frame);
                continue;
            }
            if (!cur->distance()) {
                continue;
            }
            SnakeBlock* prev = pushed;
            int d = 1;
            while (true) {
                cur->set_target(prev);
                // If we reach the end of the snake, we can pull it
                if (cur->links().size() == 1) {
                    pull_snakes_auxiliary(cur, delta_frame);
                    break;
                }
                // Progress down the snake
                for (auto link : cur->links()) {
                    if (link != prev) {
                        cur->set_distance(d++);
                        prev = cur;
                        cur = static_cast<SnakeBlock*>(link);
                        break;
                    }
                }
                // If we reach a block with a target and a shorter distance, we're done
                if (cur->target() && d >= cur->distance()) {
                    // The chain was so short that it didn't break (it was all pushed)!
                    if (cur->distance() <= 1) {
                        break;
                    }
                    // The chain was odd length; split the middle block!
                    else if (d == cur->distance()) {
                        Point pos = cur->pos();
                        take_id(pos, Layer::Solid, cur, delta_frame);
                        auto a_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, false, 1);
                        auto a = a_unique.get();
                        put(std::move(a_unique), delta_frame);
                        a->set_target(prev);
                        a->add_link(prev, delta_frame);
                        pull_snakes_auxiliary(a, delta_frame);
                        auto b_unique = std::make_unique<SnakeBlock>(pos.x, pos.y, false, 1);
                        auto b = b_unique.get();
                        put(std::move(b_unique), delta_frame);
                        b->set_target(cur->target());
                        b->add_link(cur->target(), delta_frame);
                        pull_snakes_auxiliary(b, delta_frame);
                        snake_split(cur, a, b);
                        delta_frame->push(std::make_unique<SnakeSplitDelta>(cur, a, b));
                    }
                    // The chain was even length; cut!
                    else {
                        cur->remove_link(prev, delta_frame);
                        pull_snakes_auxiliary(cur, delta_frame);
                        pull_snakes_auxiliary(prev, delta_frame);
                    }
                    break;
                }
            }
        }
    }
}

void WorldMap::snake_split(SnakeBlock* whole, SnakeBlock* half_a, SnakeBlock* half_b) {
    snakes_.erase(whole);
    snakes_.insert(half_a);
    snakes_.insert(half_b);
}

void WorldMap::snake_split_reverse(SnakeBlock* whole, SnakeBlock* half_a, SnakeBlock* half_b) {
    snakes_.insert(whole);
    snakes_.erase(half_a);
    snakes_.erase(half_b);
}

void WorldMap::pull_snakes_auxiliary(SnakeBlock* cur, DeltaFrame* delta_frame) {
    SnakeBlock* next = cur->target();
    while (next->distance()) {
        auto obj_unique = take_quiet_id(cur->pos(), Layer::Solid, cur);
        cur->set_pos(next->pos(), delta_frame);
        cur->reset_target();
        put_quiet(std::move(obj_unique));
        cur = next;
        next = cur->target();
    }
}
*/
