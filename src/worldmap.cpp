#include "worldmap.h"
#include <iostream>
#include <unordered_map>

#include "gameobject.h"
#include "delta.h"


WorldMap::WorldMap(int width, int height): width_ {width}, height_ {height}, map_ {}, seen_ {}, not_move_ {}, moved_ {}, link_update_ {}, floor_update_ {} {
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
        for (auto&& it = vec.begin(); it != vec.end(); ++it) {
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
        for (auto&& it = vec.begin(); it != vec.end(); ++it) {
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
    reset_state();
    PosIdMap result {};
    if (!move_strong_component(result, player_pos, dir))
        return;
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
    update_links(delta_frame);
}

/** Part of the movement algorithm
 * Attempts to move the object in cur_move in direction dir
 * with the knowledge that the objects in seen have been seen
 * and the objects in not_move cannot move (even if they are not walls).
 * Returns whether the object in cur_move was able to move.
 */
bool WorldMap::move_strong_component(PosIdMap& result, Point start_point, Point dir) {
    if (not_move_.count(start_point)) {
        return false;
    } else if (seen_.count(start_point) > 0) {
        return true;
    }
    PointVec to_check {start_point};
    PosIdMap cur_group {};
    // First find all strongly linked objects, because they share the same fate.
    // Either they'll all move or none of them will.
    while (!to_check.empty()) {
        Point cur_pos = to_check.back();
        to_check.pop_back();
        // It's possible (likely even, with complex groups) that we've seen this before
        if (cur_group.count(cur_pos)) {
            continue;
        }
        GameObject* cur_obj = view(cur_pos, Layer::Solid);
        cur_group.insert(std::make_pair(cur_pos, cur_obj));
        seen_.insert(cur_pos);
        for (auto& link : static_cast<Block*>(cur_obj)->get_strong_links()) {
            to_check.push_back(link->pos());
        }
    }
    // Now we determine whether they can all move.
    // If any can't, then we immediately return
    bool move_cancelled = false;
    ObjVec weak_connections;
    for (auto& cur : cur_group) {
        Point cur_pos = cur.first;
        // First make sure the object can actually move in direction dir
        Point new_pos {cur_pos.x + dir.x, cur_pos.y + dir.y};
        if (!valid(new_pos) || not_move_.count(new_pos)) {
            // We can't move forward; this whole branch is bad
            move_cancelled = true;
            result = {};
            break;
        }
        // This branch is potentially good; look at weak links
        for (auto& link : static_cast<Block*>(view(cur_pos, Layer::Solid))->get_weak_links()) {
            weak_connections.push_back(link);
        }
        if (seen_.count(new_pos)) {
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
            if (move_strong_component(branch, new_pos, dir)) {
                result.insert(branch.begin(), branch.end());
            } else {
                move_cancelled = true;
                result = branch;
                break;
            }
        }
    }
    if (move_cancelled) {
        for (auto& pos_id : cur_group) {
            result.insert(pos_id);
            not_move_.insert(pos_id.first);
        }
        return false;
    } else {
        result.insert(cur_group.begin(), cur_group.end());
        for (auto& link : weak_connections) {
            PosIdMap branch {};
            Point link_pos = link->pos();
            if (move_strong_component(branch, link_pos, dir)) {
                result.insert(branch.begin(), branch.end());
            } else {
                // Failed weak links will need to be checked for updates post-move
                link_update_.insert(link);
            }
        }
        return true;
    }
}

void WorldMap::reset_state() {
    seen_ = {};
    not_move_ = {};
    moved_ = {};
    link_update_ = {};
    floor_update_ = {};
}

void WorldMap::set_initial_state() {
    // Update all links
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            update_links_auxiliary(view(Point{x,y}, Layer::Solid), false, nullptr);
        }
    }
}

void WorldMap::update_links(DeltaFrame* delta_frame) {
    for (auto& obj : moved_) {
        update_links_auxiliary(obj, true, delta_frame);
    }
    for (auto& obj : link_update_) {
        update_links_auxiliary(obj, false, delta_frame);
    }
}

void WorldMap::update_links_auxiliary(GameObject* obj, bool save_adj, DeltaFrame* delta_frame) {
    auto pb = dynamic_cast<PushBlock*>(obj);
    if (pb && pb->sticky() != StickyLevel::None) {
        Point p = obj->pos();
        ObjSet new_links {};
        for (Point d : {Point{1,0}, Point{-1,0}, Point{0,1}, Point{0,-1}}) {
            PushBlock* adj = dynamic_cast<PushBlock*>(view(Point {p.x + d.x, p.y + d.y}, Layer::Solid));
            if (adj && pb->sticky() == adj->sticky()) {
                new_links.insert(adj);
                if (save_adj && !moved_.count(adj))
                    link_update_.insert(adj);
            }
        }
        if (pb->links() != new_links) {
            if (delta_frame)
                delta_frame->push(std::make_unique<LinkUpdateDelta>(pb, pb->links()));
            pb->set_links(new_links);
        }
    }
}
