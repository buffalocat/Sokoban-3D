#include "roommap.h"

#include "gameobject.h"
#include "delta.h"
#include "block.h"
#include "switch.h"
#include "mapfile.h"

RoomMap::RoomMap(int width, int height):
width_ {width}, height_ {height}, layers_ {}, signalers_ {} {}

bool RoomMap::valid(Point3 pos) {
    return (0 <= pos.x) && (pos.x < width_) && (0 <= pos.y) && (pos.y < height_) && (0 <= pos.z) && (pos.z < layers_.size());
}

int RoomMap::width() {
    return width_;
}

int RoomMap::height() {
    return height_;
}

int RoomMap::depth() {
    return layers_.size();
}

void RoomMap::push_full() {
    layers_.push_back(std::make_unique<FullMapLayer>(this, width_, height_));
}

void RoomMap::push_sparse() {
    layers_.push_back(std::make_unique<SparseMapLayer>(this));
}

void RoomMap::serialize(MapFileO& file) const {
    // Serialize layer types
    for (auto& layer : layers_) {
        file << layer->type();
    }
    // Serialize raw object data
    std::vector<GameObject*> rel_check;
    file << MapCode::Objects;
    for (auto& layer : layers_) {
        layer->serialize(file, rel_check);
    }
    file << ObjCode::NONE;
    // Serialize relational data
    for (auto& object : rel_check) {
        object->relation_serialize(file);
    }
    // Serialize Signalers
    for (auto& signaler : signalers_) {
        signaler->serialize(file);
    }
}

GameObject* RoomMap::view(Point3 pos) {
    return valid(pos) ? layers_[pos.z]->view(pos.h()) : nullptr;
}

void RoomMap::take(Point3 pos, DeltaFrame* delta_frame) {
    layers_[pos.z]->take(pos.h(), delta_frame);
}

std::unique_ptr<GameObject> RoomMap::take_quiet(Point3 pos) {
    return layers_[pos.z]->take_quiet(pos.h());
}

std::unique_ptr<GameObject> RoomMap::take_quiet(GameObject* obj) {
    Point3 pos {obj->pos()};
    return layers_[obj->z()]->take_quiet(pos.h());
}

void RoomMap::put(std::unique_ptr<GameObject> obj, DeltaFrame* delta_frame) {
    layers_[obj->z()]->put(std::move(obj), delta_frame);
}

void RoomMap::put_quiet(std::unique_ptr<GameObject> obj) {
    layers_[obj->z()]->put_quiet(std::move(obj));
}

void RoomMap::draw(GraphicsManager* gfx) {
    for (auto& layer : layers_) {
        layer->draw(gfx);
    }
}

void RoomMap::draw_layer(GraphicsManager* gfx, int layer) {
    layers_[layer]->draw(gfx);
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
            }
        }
    }
    check_signalers(nullptr, nullptr);
}

void RoomMap::push_signaler(std::unique_ptr<Signaler> signaler) {
    signalers_.push_back(std::move(signaler));
}

void RoomMap::check_signalers(DeltaFrame* delta_frame, std::vector<Block*>* fall_check) {
    for (auto& signaler : signalers_) {
        signaler->check_send_signal(this, delta_frame, fall_check);
    }
}
