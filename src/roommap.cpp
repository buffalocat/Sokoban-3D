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

#include "moveprocessor.h"

RoomMap::RoomMap(GameObjectArray& obj_array, int width, int height, int depth):
agents_ {}, obj_array_ {obj_array},
width_ {width}, height_ {height}, depth_ {},
layers_ {}, listeners_ {}, signalers_ {},
effects_ {std::make_unique<Effects>()} {
    // TODO: Eventually, fix the way that maplayers are chosen
    for (int i = 0; i < depth; ++i) {
        push_full();
    }
}

/*
void RoomMap::print_listeners() {
    std::cout << "\nPrinting listeners post-move!" << std::endl;
    for (auto& p : listeners_) {
        for (ObjectModifier* mod : p.second) {
            std::cout << mod->name() << " at " << p.first << std::endl;
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

    std::vector<GameObject*> rel_check_objs {};
    std::vector<ObjectModifier*> rel_check_mods {};
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
            if (!obj || obj->skip_serialization()) {
                continue;
            }
            file << obj->obj_code();
            file << it->pos();
            obj->serialize(file);
            if (ObjectModifier* mod = obj->modifier()) {
                file << mod->mod_code();
                mod->serialize(file);
                if (mod->relation_check()) {
                    rel_check_mods.push_back(mod);
                }
            } else {
                file << ModCode::NONE;
            }
            if (obj->relation_check()) {
                rel_check_objs.push_back(obj);
            }
        }
    }
    file << ObjCode::NONE;
    // Serialize Wall positions
    file << MapCode::Walls;
    // TODO: replace this with a smarter run-length encoding type of thing
    file << (int)(wall_pos.size());
    for (Point3 pos : wall_pos) {
        file << pos;
    }
    // Serialize relational data
    for (auto obj : rel_check_objs) {
        obj->relation_serialize(file);
    }
    for (auto mod : rel_check_mods) {
        mod->relation_serialize(file);
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
    obj->cleanup_on_take(this);
    obj->tangible_ = false;
    at(obj->pos_) -= obj->id_;
}

void RoomMap::just_put(GameObject* obj) {
    at(obj->pos_) += obj->id_;
    obj->setup_on_put(this);
    obj->tangible_ = true;
}

void RoomMap::take(GameObject* obj) {
    activate_listeners_at(obj->pos_);
    just_take(obj);
}

void RoomMap::put(GameObject* obj) {
    just_put(obj);
    activate_listeners_at(obj->pos_);
}

void RoomMap::take_loud(GameObject* obj, DeltaFrame* delta_frame) {
    delta_frame->push(std::make_unique<TakeDelta>(obj, this));
    take(obj);
}

void RoomMap::put_loud(GameObject* obj, DeltaFrame* delta_frame) {
    delta_frame->push(std::make_unique<PutDelta>(obj, this));
    put(obj);
}

void RoomMap::shift(GameObject* obj, Point3 dpos, DeltaFrame* delta_frame) {
    take(obj);
    obj->pos_ += dpos;
    put(obj);
    obj->set_linear_animation(dpos);
    delta_frame->push(std::make_unique<MotionDelta>(obj, dpos, this));
}

void RoomMap::batch_shift(std::vector<GameObject*> objs, Point3 dpos, DeltaFrame* delta_frame) {
    for (auto obj : objs) {
        obj->set_linear_animation(dpos);
        take(obj);
        obj->pos_ += dpos;
        put(obj);
    }
    delta_frame->push(std::make_unique<BatchMotionDelta>(std::move(objs), dpos, this));
}

// "just" means "don't do any checks, animations, deltas"
void RoomMap::just_shift(GameObject* obj, Point3 dpos) {
    just_take(obj);
    obj->pos_ += dpos;
    just_put(obj);
}

void RoomMap::just_batch_shift(std::vector<GameObject*> objs, Point3 dpos) {
    for (auto obj : objs) {
        just_take(obj);
        obj->pos_ += dpos;
        just_put(obj);
    }
}

void RoomMap::create(std::unique_ptr<GameObject> obj_unique, DeltaFrame* delta_frame) {
    GameObject* obj = obj_unique.get();
    // Need to push it into the GameObjectArray first to give it an ID
    obj_array_.push_object(std::move(obj_unique));
    put(obj);
    if (obj->is_agent()) {
        agents_.push_back(obj);
    }
    if (delta_frame) {
        delta_frame->push(std::make_unique<CreationDelta>(obj, this));
    }
}

void RoomMap::create_abstract(std::unique_ptr<GameObject> obj_unique, DeltaFrame* delta_frame) {
    if (delta_frame) {
        delta_frame->push(std::make_unique<AbstractCreationDelta>(obj_unique.get(), this));
    }
    if (obj_unique->is_agent()) {
        agents_.push_back(obj_unique.get());
    }
    obj_array_.push_object(std::move(obj_unique));
}

void RoomMap::create_wall(Point3 pos) {
    at(pos) = GLOBAL_WALL_ID;
}

void RoomMap::uncreate(GameObject* obj) {
    if (obj->is_agent()) {
        remove_agent(obj);
    }
    just_take(obj);
    obj->cleanup_on_destruction(this);
    obj_array_.destroy(obj);
}

void RoomMap::uncreate_abstract(GameObject* obj) {
    if (obj->is_agent()) {
        remove_agent(obj);
    }
    obj->cleanup_on_destruction(this);
    obj_array_.destroy(obj);
}

void RoomMap::destroy(GameObject* obj, DeltaFrame* delta_frame) {
    if (obj->is_agent()) {
        remove_agent(obj);
    }
    obj->cleanup_on_destruction(this);
    take(obj);
    if (delta_frame) {
        delta_frame->push(std::make_unique<DeletionDelta>(obj, this));
    }
}

void RoomMap::undestroy(GameObject* obj) {
    just_put(obj);
    if (obj->is_agent()) {
        agents_.push_back(obj);
    }
    obj->setup_on_undestruction(this);
}

void RoomMap::remove_agent(GameObject* obj) {
    agents_.erase(std::remove(agents_.begin(), agents_.end(), obj), agents_.end());
}

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
    // Using a "fake" DeltaFrame just this once means we
    // don't have to do a bunch of redundant checks during play
    DeltaFrame dummy_df {};
    MoveProcessor mp = MoveProcessor(nullptr, this, &dummy_df);
    for (auto& layer : layers_) {
        for (auto it = layer->begin_iter(); !it->done(); it->advance()) {
            GameObject* obj = obj_array_[it->id()];
            if (!obj) {
                continue;
            }
            if (obj->gravitable_) {
                mp.add_to_fall_check(obj);
            }
            if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj)) {
                sb->check_add_local_links(this, &dummy_df);
            }
            if (ObjectModifier* mod = obj->modifier()) {
                activate_listener_of(mod);
            }
        }
    }
    // In editor mode, use effects of snakes, but not switches or other things.
    if (editor_mode) {
        return;
    }
    mp.perform_switch_checks();
    mp.try_fall_step();
}

// This function does just one of the things that set_initial_state does
// But it's useful for making the SnakeTab convenient!
void RoomMap::initialize_automatic_snake_links() {
    DeltaFrame dummy_df {};
    for (auto& layer : layers_) {
        for (auto it = layer->begin_iter(); !it->done(); it->advance()) {
            if (SnakeBlock* sb = dynamic_cast<SnakeBlock*>(obj_array_[it->id()])) {
                sb->check_add_local_links(this, &dummy_df);
            }
        }
    }
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

void RoomMap::remove_signaler(Signaler* rem) {
    signalers_.erase(std::remove_if(signalers_.begin(), signalers_.end(),
        [rem](std::unique_ptr<Signaler>& sig) {return sig.get() == rem;}), signalers_.end());
}


void RoomMap::make_fall_trail(GameObject* block, int height, int drop) {
    effects_->push_trail(block, height, drop);
}
