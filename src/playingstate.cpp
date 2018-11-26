#include "playingstate.h"

#include <unistd.h>

#include "gameobject.h"
#include "block.h"
#include "player.h"
#include "room.h"
#include "roommap.h"
#include "moveprocessor.h"
#include "door.h"
#include "mapfile.h"

PlayingState::PlayingState(std::string name, Point3 pos, bool testing):
GameState(), loaded_rooms_ {}, room_ {}, player_ {},
undo_stack_ {MAX_UNDO_DEPTH},
keyboard_cooldown_ {0}, testing_ {testing} {
    activate_room(name);
    init_player(pos);
}

PlayingState::~PlayingState() = default;

void PlayingState::init_player(Point3 pos) {
    RidingState rs;
    Block* block = dynamic_cast<Block*>(room_->room_map()->view({pos.x, pos.y, pos.z - 1}));
    if (block) {
        if (block->car()) {
            rs = RidingState::Riding;
        } else {
            rs = RidingState::Bound;
        }
    } else {
        rs = RidingState::Free;
    }
    auto player = std::make_unique<Player>(pos, rs);
    player_ = player.get();
    room_->room_map()->put_quiet(std::move(player));
}

void PlayingState::main_loop() {
    auto delta_frame = std::make_unique<DeltaFrame>();
    handle_input(delta_frame.get());
    room_->set_cam_target(player_->pos());
    room_->draw(gfx_, player_->pos(), false);
    undo_stack_.push(std::move(delta_frame));
}

void PlayingState::handle_input(DeltaFrame* delta_frame) {
    if (keyboard_cooldown_ > 0) {
        --keyboard_cooldown_;
        return;
    }
    keyboard_cooldown_ = MAX_COOLDOWN;
    RoomMap* room_map = room_->room_map();
    for (auto p : MOVEMENT_KEYS) {
        if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
            MoveProcessor(player_, room_map, p.second).try_move(delta_frame);
            Door* door = static_cast<Door*>(room_map->view(player_->shifted_pos({0,0,-1})));
            // When doors are switchable, check for state too!
            if (door && door->dest() && door->state()) {
                use_door(door->dest(), delta_frame);
            }
            return;
        }
    }
    if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
        if (undo_stack_.pop()) {
            if (player_) {
                room_->set_cam_pos(player_->pos());
            }
            return;
        }
    } else if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS) {
        player_->toggle_riding(room_map, delta_frame);
        return;
    } else if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
        Block* car = player_->get_car(room_map, false);
        if (car) {
            if (car->cycle_color(false)) {
                delta_frame->push(std::make_unique<ColorChangeDelta>(car));
            }
            return;
        }
    }
    keyboard_cooldown_ = 0;
}

bool PlayingState::activate_room(std::string name) {
    if (!loaded_rooms_.count(name)) {
        if (!load_room(name)) {
            return false;
        }
    }
    room_ = loaded_rooms_[name].get();
    return true;
}

bool PlayingState::load_room(std::string name) {
    std::string path;
    // This will be much more complicated when save files are a thing
    path = MAPS_TEMP + name + ".map";
    if (access(path.c_str(), F_OK) == -1) {
        path = MAPS_MAIN + name + ".map";
        if (access(path.c_str(), F_OK) == -1) {
            return false;
        }
    }
    MapFileI file {path};
    auto room = std::make_unique<Room>(name);
    room->load_from_file(file);
    // Load dynamic component!
    room->room_map()->set_initial_state(false);
    loaded_rooms_[name] = std::move(room);
    return true;
}

void PlayingState::use_door(MapLocation* dest, DeltaFrame* delta_frame) {
    if (!loaded_rooms_.count(dest->name)) {
        load_room(dest->name);
    }
    Room* dest_room = loaded_rooms_[dest->name].get();
    RoomMap* cur_map = room_->room_map();
    RoomMap* dest_map = dest_room->room_map();
    if (dest_map->view(dest->pos)) {
        return;
    }
    Block* car = player_->get_car(cur_map, true);
    if (car) {
        if (dest_map->view(dest->pos)) {
            return;
        }
    }
    delta_frame->push(std::make_unique<DoorMoveDelta>(this, room_, player_->pos()));
    room_ = dest_room;
    auto player_unique = cur_map->take_quiet(player_);
    if (car) {
        auto car_unique = cur_map->take_quiet(car);
        car->set_pos(dest->pos);
        dest_map->put_quiet(std::move(car_unique));
    }
    player_->set_pos(dest->pos);
    dest_map->put_quiet(std::move(player_unique));
}
