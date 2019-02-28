#include "gate.h"

#include "point.h"
#include <memory>
#include "gatebody.h"
#include "mapfile.h"
#include "roommap.h"

#include "moveprocessor.h"
#include "graphicsmanager.h"

// Gates should be initialized down in case they are "covered" at load time
Gate::Gate(GameObject* parent, GateBody* body, int color, bool def, bool active): Switchable(parent, def, active), color_ {color}, body_ {body} {}

Gate::~Gate() {}

ModCode Gate::mod_code() {
    return ModCode::Gate;
}

void Gate::serialize(MapFileO& file) {
    file << color_ << default_ << active_;
}

void Gate::deserialize(MapFileI& file, RoomMap* room_map, GameObject* parent) {
    unsigned char b[3];
    file.read(b, 3);
    auto gate = std::make_unique<Gate>(parent, nullptr, b[0], b[1], b[2]);
    auto gate_body_unique = std::make_unique<GateBody>(gate.get());
    gate->body_ = gate_body_unique.get();
    room_map->create_abstract(std::move(gate_body_unique));
    parent->set_modifier(std::move(gate));
}

void Gate::collect_sticky_links(RoomMap* room_map, Sticky, std::vector<GameObject*>& to_check) {
    if (state()) {
        to_check.push_back(body_);
    }
}

bool Gate::can_set_state(bool state, RoomMap* room_map) {
    // You can always set state to false, but setting it to true requires there be
    // nothing above the gate
    return !state || (room_map->view(pos_above()) == nullptr);
}

void Gate::apply_state_change(RoomMap* room_map, MoveProcessor* mp) {
    if (state()) {
        room_map->put(body_);
    } else {
        room_map->take(body_);
        // TODO: investigate/fix optionality of fall_check (original solution: use pointer, not reference)
        if (mp) {
            GameObject* above = room_map->view(pos() + Point3{0,0,2});
            if (above && above->gravitable_) {
                mp->add_to_fall_check(above);
            }
        }
    }
}

void Gate::map_callback(RoomMap* room_map, DeltaFrame* delta_frame, MoveProcessor* mp) {
    check_waiting(room_map, delta_frame, mp);
}

void Gate::setup_on_put(RoomMap* room_map) {
    room_map->add_listener(this, pos_above());
    room_map->activate_listener_of(this);
}

void Gate::cleanup_on_take(RoomMap* room_map) {
    room_map->remove_listener(this, pos_above());
}

void Gate::draw(GraphicsManager* gfx, FPoint3 p) {
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z + 0.5f, p.y));
    model = glm::scale(model, glm::vec3(0.8f, 0.1f, 0.8f));
    gfx->set_tex(Texture::Blank);
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    gfx->draw_cube();
}

std::unique_ptr<ObjectModifier> Gate::duplicate(GameObject* parent) {
    // TODO: copy the GateBody too!!!
    auto dup = std::make_unique<Gate>(*this);
    dup->parent_ = parent;
    dup->connect_to_signalers();
    return std::move(dup);
}
