#include "pressswitch.h"

#include "mapfile.h"
#include "roommap.h"
#include "delta.h"
#include "graphicsmanager.h"

PressSwitch::PressSwitch(GameObject* parent, unsigned char color, bool persistent, bool active):
Switch(parent, persistent, active), color_ {color} {}

PressSwitch::~PressSwitch() {}

ModCode PressSwitch::mod_code() {
    return ModCode::PressSwitch;
}

void PressSwitch::serialize(MapFileO& file) {
    file << color_;
    file << (persistent_ | (active_ << 1));
}

/*
GameObject* PressSwitch::deserialize(MapFileI& file) {
    Point3 pos {file.read_point3()};
    unsigned char b[2];
    file.read(b, 2);
    return new PressSwitch(pos, b[0], b[1] & 1, b[1] & 2);
}
*/

void PressSwitch::check_send_signal(RoomMap* room_map, DeltaFrame* delta_frame) {
    if (active_ && persistent_) {
        return;
    }
    if (should_toggle(room_map)) {
        if (delta_frame) {
            delta_frame->push(std::make_unique<SwitchToggleDelta>(this));
        }
        toggle();
    }
}

bool PressSwitch::should_toggle(RoomMap* room_map) {
    return active_ ^ (room_map->view(pos_above()) != nullptr);
}

void PressSwitch::map_callback(RoomMap* room_map, DeltaFrame* delta_frame) {
    check_send_signal(room_map, delta_frame);
}

void PressSwitch::setup_on_put(RoomMap* room_map) {
    room_map->add_listener(this, pos_above());
    room_map->activate_listener_of(this);
}

void PressSwitch::cleanup_on_take(RoomMap* room_map) {
    room_map->remove_listener(this, pos_above());
}

void PressSwitch::draw(GraphicsManager* gfx) {
    Point3 p = pos();
    gfx->set_model(glm::translate(glm::mat4(), glm::vec3(p.x, p.z, p.y)));
    gfx->set_color(COLORS[GREY]);
    gfx->draw_cube();
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(p.x, p.z + 0.5, p.y));
    model = glm::scale(model, glm::vec3(0.9f, 0.1f, 0.9f));
    gfx->set_model(model);
    gfx->set_color(COLORS[color_]);
    if (persistent_) {
        if (active_) {
            gfx->set_tex(Texture::SwitchDown);
        } else {
            gfx->set_tex(Texture::SwitchUp);
        }
    } else {
        gfx->set_tex(Texture::Cross);
    }
    gfx->draw_cube();
    gfx->set_tex(Texture::Blank);
}
