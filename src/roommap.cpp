#include "roommap.h"

#include <algorithm>

#include "gameobjectarray.h"

#include "gameobject.h"
#include "delta.h"

#include "snakeblock.h"
#include "switch.h"
#include "signaler.h"
#include "mapfile.h"

#include "objectmodifier.h"

#include "maplayer.h"

#include "effects.h"

RoomMap::RoomMap(GameObjectArray& obj_array, int width, int height, int depth):
obj_array_ {obj_array}, width_ {width}, height_ {height}, depth_ {},
layers_ {}, listeners_ {}, signalers_ {},
effects_ {std::make_unique<Effects>()} {
    // TODO: Eventually, fix the way that maplayers are chosen
    for (int i = 0; i < depth; ++i) {
        push_full();
    }
}

/*
#include <iostream>

void RoomMap::print_snakes() {
    for (auto& layer : layers_) {
        for (auto it = layer->begin_iter(); !it->done(); it->advance()) {
            SnakeBlock* obj = dynamic_cast<SnakeBlock*>(obj_array_[it->id()]);
            if (obj) {
                std::cout << "snake at " << obj->pos_ << "with d = " << obj->distance_ << std::endl;
            }
        }
    }
}
*/

RoomMap::~RoomMap() {}

bool RoomMap::valid(Point3 pos) {
    return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_) && (0 <= pos.z) && ((unsigned int)(pos.z) < layers_.size());
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
            if (ObjectModifier* mod = obj->modifier()) {
                file << mod->mod_code();
                mod->serialize(file);
            } else {
                file << ModCode::NONE;
            }
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

// Pretend that every out-of-bounds "object" is a Wall, unless it's below the map
GameObject* RoomMap::view(Point3 pos) {
    if (pos.z < 0) {
        return nullptr;
    } else if (valid(pos)) {
        return obj_array_[layers_[pos.z]->at(pos.h())];
    } else {
        return obj_array_[GLOBAL_WALL_ID];
    }
}

void RoomMap::just_take(GameObject* obj) {
    at(obj->pos_) -= obj->id_;
}

void RoomMap::just_put(GameObject* obj) {
    at(obj->pos_) += obj->id_;
}

void RoomMap::take(GameObject* obj) {
    activate_listeners_at(obj->pos_);
    obj->cleanup_on_take(this);
    at(obj->pos_) -= obj->id_;
}

void RoomMap::put(GameObject* obj) {
    at(obj->pos_) += obj->id_;
    obj->setup_on_put(this);
    activate_listeners_at(obj->pos_);
}

void RoomMap::shift(GameObject* obj, Point3 dpos, DeltaFrame* delta_frame) {
    take(obj);
    obj->pos_ += dpos;
    put(obj);
    // NOTE: putting animation here is a bit of a hack, but it works!
    // Animation only gets set if we're not undoing!
    if (delta_frame) {
        obj->set_linear_animation(dpos);
        delta_frame->push(std::make_unique<MotionDelta>(obj, dpos, this));
    }
}

void RoomMap::batch_shift(std::vector<GameObject*> objs, Point3 dpos, DeltaFrame* delta_frame) {
    for (auto obj : objs) {

        if (delta_frame) {
            obj->set_linear_animation(dpos);
        }
        take(obj);
        obj->pos_ += dpos;
        put(obj);
    }
    if (delta_frame) {
        delta_frame->push(std::make_unique<BatchMotionDelta>(std::move(objs), dpos, this));
    }
}

void RoomMap::create(std::unique_ptr<GameObject> obj) {
    GameObject* raw = obj.get();
    obj_array_.push_object(std::move(obj));
    at(raw->pos_) += raw->id_;
}

void RoomMap::create(std::unique_ptr<GameObject> obj, DeltaFrame* delta_frame) {
    GameObject* raw = obj.get();
    obj_array_.push_object(std::move(obj));
    at(raw->pos_) += raw->id_;
    delta_frame->push(std::make_unique<CreationDelta>(raw, this));
}

void RoomMap::create_wall(Point3 pos) {
    at(pos) = GLOBAL_WALL_ID;
}

void RoomMap::uncreate(GameObject* obj) {
    just_take(obj);
    obj->cleanup_on_destruction(this);
    obj_array_.destroy(obj);
}

void RoomMap::destroy(GameObject* obj) {
    obj->cleanup_on_destruction(this);
    obj->alive_ = false;
    take(obj);
}

void RoomMap::destroy(GameObject* obj, DeltaFrame* delta_frame) {
    obj->cleanup_on_destruction(this);
    obj->alive_ = false;
    take(obj);
    if (delta_frame) {
        delta_frame->push(std::make_unique<DeletionDelta>(obj, this));
    }
}

void RoomMap::undestroy(GameObject* obj) {
    just_put(obj);
    obj->alive_ = true;
    obj->setup_on_undestruction(this);
}

// TODO (maybe): Consider allowing for a general callback function here
void RoomMap::add_listener(ObjectModifier* obj, Point3 pos) {
    listeners_[pos].push_back(obj);
}

void RoomMap::remove_listener(ObjectModifier* obj, Point3 pos) {
    auto& cur_lis = listeners_[pos];
    cur_lis.erase(std::remove(cur_lis.begin(), cur_lis.end(), obj), cur_lis.end());
    if (cur_lis.size() == 0) {
        listeners_.erase(pos);
    }
}

void RoomMap::activate_listener_of(ObjectModifier* obj) {
    activated_listeners_.insert(obj);
}

void RoomMap::activate_listeners_at(Point3 pos) {
    if (listeners_.count(pos)) {
        auto& cur_lis = listeners_[pos];
        activated_listeners_.insert(cur_lis.begin(), cur_lis.end());
    }
}

void RoomMap::alert_activated_listeners(DeltaFrame* delta_frame, MoveProcessor* mp) {
    for (ObjectModifier* obj : activated_listeners_) {
        obj->map_callback(this, delta_frame, mp);
    }
}

void RoomMap::draw(GraphicsManager* gfx, float angle) {
    for (auto& layer : layers_) {
        for (auto it = layer->begin_iter(); !it->done(); it->advance()) {
            int id = it->id();
            if (id > GLOBAL_WALL_ID) {
                obj_array_[id]->draw(gfx);
            }
        }
    }
    // TODO: draw walls!
    effects_->sort_by_distance(angle);
    effects_->draw(gfx);
}

void RoomMap::draw_layer(GraphicsManager* gfx, int z) {
    MapLayer* layer = layers_[z].get();
    for (auto it = layer->begin_iter(); !it->done(); it->advance()) {
        int id = it->id();
        if (id > GLOBAL_WALL_ID) {
            obj_array_[id]->draw(gfx);
        }
    }
}

void RoomMap::set_initial_state(bool editor_mode) {
    /*
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
                // This doesn't even make sense in the current model!
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
    // TODO: fix (well, fix this whole method actually)
    check_signalers(nullptr, nullptr);*/
}

// The room keeps track of some things which must be forgotten after a move or undo
void RoomMap::reset_local_state() {
    activated_listeners_ = {};
}

void RoomMap::push_signaler(std::unique_ptr<Signaler> signaler) {
    signalers_.push_back(std::move(signaler));
}

// NOTE: this function breaks the "locality" rule, but it's probably not a big deal.
void RoomMap::check_signalers(DeltaFrame* delta_frame, MoveProcessor* mp) {
    for (auto& signaler : signalers_) {
        signaler->check_send_signal(this, delta_frame, mp);
    }
}

void RoomMap::remove_from_signalers(ObjectModifier* obj) {
    if (!obj) {
        return;
    }
    signalers_.erase(std::remove_if(signalers_.begin(), signalers_.end(),
                                    [obj](std::unique_ptr<Signaler>& sig) {return sig->remove_object(obj);}), signalers_.end());
}

void RoomMap::make_fall_trail(GameObject* block, int height, int drop) {
    effects_->push_trail(block, height, drop);
}
