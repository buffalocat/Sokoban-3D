#include "roommap.h"

#include "gameobject.h"
#include "delta.h"
#include "block.h"
#include "snakeblock.h"
#include "switch.h"
#include "mapfile.h"

#include <algorithm>

RoomMap::RoomMap(GameObjectArray& obj_array, int width, int height, int depth):
obj_array_ {obj_array}, width_ {width}, height_ {height}, depth_ {depth},
layers_ {}, listeners_ {}, signalers_ {},
effects_ {std::make_unique<Effects>()} {}

bool RoomMap::valid(Point3 pos) {
    return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_) && (0 <= pos.z) && (pos.z < layers_.size());
}

void RoomMap::push_full() {
    layers_.push_back(std::make_unique<FullMapLayer>(this, width_, height_, depth_));
    ++depth_;
}

void RoomMap::push_sparse() {
    layers_.push_back(std::make_unique<SparseMapLayer>(this, depth_));
    ++depth_;
}

void RoomMap::serialize(MapFileO& file) const {
    // Serialize layer types
    for (auto& layer : layers_) {
        file << layer->type();
    }

    std::vector<GameObject*> rel_check {};
    std::vector<Point3> wall_pos {};
    // Serialize raw object data
    file << MapCode::Objects;
    for (auto& layer : layers_) {
        for (auto it = layer->begin_iter(); !it->done(); it->advance()) {
            if (it->id() == GLOBAL_WALL_ID) {
                wall_pos.push_back(it->pos());
                continue;
            }
            GameObject* obj = obj_array_[it->id()];
            if (!obj || obj->obj_code() == ObjCode::Player) {
                continue;
            }
            file << obj->obj_code();
            file << it->pos();
            obj->serialize(file);
            if (obj->relation_check()) {
                rel_check.push_back(obj);
            }
        }
    }
    file << ObjCode::NONE;
    // Serialize Wall positions
    file << MapCode::Walls;
    file << (int)(wall_pos.size());
    for (Point3 pos : wall_pos) {
        file << pos;
    }
    // Serialize relational data
    for (auto& object : rel_check) {
        object->relation_serialize(file);
    }
    // Serialize Signalers
    for (auto& signaler : signalers_) {
        signaler->serialize(file);
    }
}

int& RoomMap::at(Point3 pos) {
    return layers_[pos.z]->at(pos.h());
}

// Pretend that every out-of-bounds "object" is a Wall.
GameObject* RoomMap::view(Point3 pos) {
    return valid(pos) ? obj_array_[layers_[pos.z]->at(pos.h())] : obj_array_[GLOBAL_WALL_ID];
}

void RoomMap::take(GameObject* obj) {
    activate_listeners(obj->pos_);
    obj->cleanup_on_take(this);
    at(obj->pos_) -= obj->id_;
}

void RoomMap::put(GameObject* obj) {
    activate_listeners(obj->pos_);
    obj->setup_on_put(this);
    at(obj->pos_) += obj->id_;
}

void RoomMap::shift(GameObject* obj, Point3 dpos, DeltaFrame* delta_frame) {
    take(obj);
    obj->pos_ += dpos;
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(obj, dpos, this));
    }
    put(obj);
}

void RoomMap::batch_shift(std::vector<GameObject*> objs, Point3 dpos, DeltaFrame* delta_frame) {
    for (auto obj : objs) {
        take(obj);
        obj->pos_ += dpos;
        put(obj);
    }
    if (delta_frame) {
        delta_frame->push(std::make_unique<MotionDelta>(obj, dpos, this));
    }
}

void RoomMap::create(std::unique_ptr<GameObject> obj, Point3 pos, DeltaFrame* delta_frame) {
    obj_array_->push_object(std::move(obj));
    at(pos) = obj->id_;
}

// TODO (maybe): Consider allowing for a general callback function here
void RoomMap::add_listener(GameObject* obj, MapCallback callback, Point3 pos) {
    listeners_[pos].push_back(std::make_pair(obj, callback));
}

void RoomMap::alert_activated_listeners(DeltaFrame* delta_frame) {
    for (auto& p : activated_listeners_) {
        p.first->p.second(this, delta_frame);
    }
}

void RoomMap::remove_listener(GameObject* obj, Point3 pos) {
    auto& cur_lis = listeners_[pos];
    cur_lis.erase(std::remove_if(cur_lis.begin(), cur_lis.end(),
                                 [](auto p) {return p.first == obj}), cur_lis.end());
    if (cur_listeners.size() == 0) {
        listeners_.erase(pos);
    }
}

void RoomMap::activate_listeners(Point3 pos) {
    if (listeners_.count(pos)) {
        auto& cur_lis = listeners_[pos];
        activated_listeners_.insert(cur_lis.begin(), cur_lis.end());
    }
}

void RoomMap::draw(GraphicsManager* gfx, float angle) {
    for (auto& layer : layers_) {
        for (auto it = layer.begin_iter(); !it->done(); it->advance()) {
            int id = it->id();
            if (id) {
                objs[id]->draw(gfx, it->pos());
            }
        }
    }
    effects_->sort_by_distance(angle);
    effects_->draw(gfx);
}

void RoomMap::set_initial_state(bool editor_mode) {
    // Gates don't get activated in editor mode!
    // When we add in other things we won't do the early return though.
    if (editor_mode) {
        return;
    }
    for (int x = 0; x != width_; ++x) {
        for (int y = 0; y != height_; ++y) {
            for (int z = 0; z != layers_.size(); ++z) {
                GameObject* obj = view({x,y,z});
                auto gate = dynamic_cast<Gate*>(obj);
                if (gate) {
                    gate->check_waiting(this, nullptr);
                    continue;
                }
                auto sw = dynamic_cast<Switch*>(obj);
                if (sw) {
                    sw->check_send_signal(this, nullptr);
                }
                auto sb = dynamic_cast<SnakeBlock*>(obj);
                if (sb) {
                    sb->check_add_local_links(this, nullptr);
                }
            }
        }
    }
    check_signalers(nullptr, nullptr);
}

// The room keeps track of some things which must be forgotten after a move or undo
void RoomMap::reset_local_state() {
    activated_listeners_ = {};
    snakes_to_update_ = {};
}

void RoomMap::push_signaler(std::unique_ptr<Signaler> signaler) {
    signalers_.push_back(std::move(signaler));
}

void RoomMap::check_signalers(DeltaFrame* delta_frame, std::vector<Block*>* fall_check) {
    for (auto& signaler : signalers_) {
        signaler->check_send_signal(this, delta_frame, fall_check);
    }
}

void RoomMap::remove_from_signalers(GameObject* obj) {
    signalers_.erase(std::remove_if(signalers_.begin(), signalers_.end(),
                                    [obj](std::unique_ptr<Signaler>& sig) {return sig->remove_object(obj);}), signalers_.end());
}

void RoomMap::make_fall_trail(Block* block, int height, int drop) {
    effects_->push_trail(block, height, drop);
}
