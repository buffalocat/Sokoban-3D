#include "gate.h"

#include "point.h"
#include <memory>
#include "gatebody.h"
#include "mapfile.h"
#include "roommap.h"

#include "graphicsmanager.h"

// Gates should be initialized down in case they are "covered" at load time
Gate::Gate(GameObject* parent, GateBody* body, bool def): Switchable(parent, def, false), body_ {body} {}

Gate::~Gate() {}

ModCode Gate::mod_code() {
    return ModCode::Gate;
}

void Gate::serialize(MapFileO& file) {
    file << default_;
}

/*GameObject* Gate::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[1];
    file.read(b, 1);
    return new Gate(pos, b[0]);
}*/

bool Gate::can_set_state(bool state, RoomMap* room_map) {
    // You can always set state to false, but setting it to true requires there be
    // nothing above the gate
    return !state || (room_map->view(pos() + Point3{0,0,1}) == nullptr);
}

void Gate::apply_state_change(RoomMap* room_map, std::vector<GameObject*>& fall_check) {
    if (state()) {
        room_map->put(body_);
    } else {
        room_map->take(body_);
        // TODO: investigate/fix optionality of fall_check (original solution: use pointer, not reference)
        if (true) {
            GameObject* above = room_map->view(pos() + Point3{0,0,2});
            if (above->gravitable_) {
                fall_check.push_back(above);
            }
        }
    }
}

// TODO: Fix the listener system!!
void Gate::setup_on_put(RoomMap* room_map) {
    //room_map->add_listener(this, &Gate::check_waiting, pos() + Point3{0,0,1});
    //room_map->activate_listener(this);
}

void Gate::cleanup_on_take(RoomMap* room_map) {
    //room_map->remove_listener(this, pos_ + {0,0,1});
}

void Gate::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(COLORS[LIGHT_GREY]);
    gfx->draw_cube();
}
