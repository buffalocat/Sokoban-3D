#include "worldmap.h"
#include <iostream>
#include <unordered_map>

#include "gameobject.h"
#include "delta.h"


WorldMap::WorldMap(int width, int height): width_ {width}, height_ {height}, map_ {} {
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
            delta_frame->push(static_cast<std::unique_ptr<Delta>>(std::make_unique<DeletionDelta>(std::move(vec.back()))));
            vec.pop_back();
        }
    }
}

void WorldMap::take_id(Point pos, Layer layer, GameObject* id, DeltaFrame* delta_frame) {
    if (valid(pos)) {
        auto &vec = map_[pos.x][pos.y][static_cast<unsigned int>(layer)];
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            if ((*it).get() == id) {
                delta_frame->push(static_cast<std::unique_ptr<Delta>>(std::make_unique<DeletionDelta>(std::move(*it))));
                vec.erase(it);
                return;
            }
        }
        throw "Tried to take an object that wasn't there!";
    }
    throw "Tried to take an object from an invalid location!";
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
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            if ((*it).get() == id) {
                auto obj = std::move(*it);
                vec.erase(it);
                return obj;
            }
        }
        throw "Tried to (quietly) take an object that wasn't there!";
    }
    throw "Tried to (quietly) take an object from an invalid location!";
}

void WorldMap::put(std::unique_ptr<GameObject> object, DeltaFrame* delta_frame) {
    Point pos = object->pos();
    if (valid(pos)) {
        delta_frame->push(std::make_unique<CreationDelta>(object.get()));
        map_[pos.x][pos.y][static_cast<unsigned int>(object->layer())].push_back(std::move(object));
    } else {
        throw "Tried to put an object in an invalid location!";
    }
}

void WorldMap::put_quiet(std::unique_ptr<GameObject> object) {
    Point pos = object->pos();
    if (valid(pos)) {
        map_[pos.x][pos.y][static_cast<unsigned int>(object->layer())].push_back(std::move(object));
    } else {
        throw "Tried to (quietly) put an object in an invalid location!";
    }
}

void WorldMap::draw(Shader* shader) {
    for (auto& column : map_) {
        for (auto& cell : column) {
            for (auto& layer : cell) {
                for (auto& object : layer) {
                    object->draw(shader);
                }
            }
        }
    }
}


// Note: many things are predicated on the assumption that, in any consistent game state,
// certain layers allow at most one object per MapCell.  These include Solid and Player.
void WorldMap::move_solid(Point player_pos, Point dir, DeltaFrame* delta_frame) {
    std::cout << "\n\nTrying to move" << std::endl;
    PosIdMap seen {};
    PosIdMap not_move {};
    PosIdMap result {};
    ObjSet broken {};
    if (!move_strong_component(seen, not_move, result, broken, player_pos, dir)) return;
    Point pos;
    GameObject* obj;
    for (auto pos_id : result) {
        std::tie(pos, obj) = pos_id;
        // Hardcode layer for now - it might be more complicated later.
        auto obj_unique = take_quiet_id(pos, Layer::Solid, obj);
        static_cast<Block*>(obj)->shift_pos(dir, delta_frame);
        put_quiet(std::move(obj_unique));
    }
    for (auto pos_id : seen) {
        PushBlock* pb = dynamic_cast<PushBlock*>(pos_id.second);
        if (pb)
            pb->update_links(this, true, delta_frame);
    }
    for (auto obj : broken) {
        PushBlock* pb = dynamic_cast<PushBlock*>(obj);
        if (pb)
            pb->update_links(this, false, delta_frame);
    }
}

/** Part of the movement algorithm
 * Attempts to move the object in cur_move in direction dir
 * with the knowledge that the objects in seen have been seen
 * and the objects in not_move cannot move (even if they are not walls).
 * Returns whether the object in cur_move was able to move.
 */
bool WorldMap::move_strong_component(PosIdMap& seen, PosIdMap& not_move, PosIdMap& result, ObjSet& broken, Point start_point, Point dir) {
    if (not_move.count(start_point) > 0) {
        return false;
    } else if (seen.count(start_point) > 0) {
        return true;
    }
    std::cout << "Starting a Component!" << std::endl;
    std::vector<Point> to_check {start_point};
    PosIdMap cur_group {};
    // First find all strongly linked objects, because they share the same fate.
    // Either they'll all move or none of them will.
    while (!to_check.empty()) {
        Point cur_pos = to_check.back();
        to_check.pop_back();
        // It's possible (likely even, with complex groups) that we've seen this before
        if (cur_group.count(cur_pos) > 0) {
            continue;
        }
        Block* cur_obj = static_cast<Block*>(view(cur_pos, Layer::Solid));
        auto cur_pos_id = std::make_pair(cur_pos, cur_obj);
        seen.insert(cur_pos_id);
        cur_group.insert(cur_pos_id);
        for (auto link : cur_obj->get_strong_links()) {
            to_check.push_back(link.second->pos());
        }
    }
    // Now we determine whether they can all move.
    // If any can't, then we immediately return
    bool move_cancelled = false;
    PosIdVec weak_connections;
    for (auto cur_pos_id : cur_group) {
        Point cur_pos;
        GameObject* cur_obj;
        std::tie(cur_pos, cur_obj) = cur_pos_id;
        // First make sure the object can actually move in direction dir
        Point new_pos {cur_pos.x + dir.x, cur_pos.y + dir.y};
        if (!valid(new_pos) || not_move.count(new_pos) > 0) {
            // We can't move forward; this whole branch is bad
            move_cancelled = true;
            result = {};
            break;
        }
        // This branch is potentially good; look at weak links
        for (auto link : static_cast<Block*>(view(cur_pos, Layer::Solid))->get_weak_links()) {
            Point rel_pos, link_pos;
            GameObject* link_obj;
            std::tie(rel_pos, link_obj) = link;
            link_pos = {cur_pos.x + rel_pos.x, cur_pos.y + rel_pos.y};
            weak_connections.push_back(std::make_pair(link_pos, link_obj));
        }
        if (seen.count(new_pos) > 0) {
            // If it was in seen but not in not_move, then it has already been verified ok
            continue;
        }
        GameObject* new_obj = view(new_pos, Layer::Solid);
        if (new_obj) {
            if (new_obj->wall()) {
                move_cancelled = true;
                result = {};
                break;
            }
            // If this branch is good, insert it into the current result
            // If it's bad, we ONLY return the bad branch
            PosIdMap branch {};
            if (move_strong_component(seen, not_move, branch, broken, new_pos, dir)) {
                result.insert(branch.begin(), branch.end());
            } else {
                move_cancelled = true;
                result = branch;
                break;
            }
        }
    }
    if (move_cancelled) {
        result.insert(cur_group.begin(), cur_group.end());
        not_move.insert(cur_group.begin(), cur_group.end());
        return false;
    } else {
        result.insert(cur_group.begin(), cur_group.end());
        for (auto cur_pos_id : weak_connections) {
            PosIdMap branch {};
            if (move_strong_component(seen, not_move, branch, broken, cur_pos_id.first, dir)) {
                result.insert(branch.begin(), branch.end());
            } else {
                broken.insert(cur_pos_id.second);
            }
        }
        return true;
    }
}




void WorldMap::initialize_links(bool check_all) {
    if (check_all) {

    }
}

void WorldMap::init_sticky() {
    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            PushBlock* obj = dynamic_cast<PushBlock*>(view(Point {i,j}, Layer::Solid));
            if (obj)
                obj->update_links(this, false, nullptr);
        }
    }
}
