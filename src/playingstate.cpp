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
GameState(), loaded_rooms_ {}, objs_ {std::make_unique<GameObjectArray>()},
move_processor_ {}, room_ {}, player_ {},
undo_stack_ {MAX_UNDO_DEPTH},
testing_ {testing} {
    activate_room(name);
    init_player(pos);
    room_->room_map()->set_initial_state(false);
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
    if (!move_processor_) {
        delta_frame_ = std::make_unique<DeltaFrame>();
    }
    handle_input();
    room_->draw(gfx_, player_, false, false);
    if (!move_processor_) {
        undo_stack_.push(std::move(delta_frame_));
    }
}

void PlayingState::handle_input() {
    static int input_cooldown = 0;
    static int undo_combo = 0;
    if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
        if (input_cooldown == 0) {
            ++undo_combo;
            if (undo_combo < UNDO_COMBO_FIRST) {
                input_cooldown = UNDO_COOLDOWN_FIRST;
            } else if (undo_combo < UNDO_COMBO_SECOND) {
                input_cooldown = UNDO_COOLDOWN_SECOND;
            } else {
                input_cooldown = UNDO_COOLDOWN_FINAL;
            }
            if (move_processor_) {
                move_processor_->abort();
                move_processor_.reset(nullptr);
                room_->map_->reset_local_sate();
                delta_frame_->revert();
                delta_frame_ = std::make_unique<DeltaFrame>();
                if (player_) {
                    room_->set_cam_pos(player_->pos());
                }
            } else if (undo_stack_.non_empty()) {
                undo_stack_.pop();
                if (player_) {
                    room_->set_cam_pos(player_->pos());
                }
            }
            return;
        }
    } else {
        undo_combo = 0;
    }
    bool ignore_input = false;
    if (input_cooldown > 0) {
        --input_cooldown;
        ignore_input = true;
    }
    // Ignore all other input if an animation is occurring
    if (move_processor_) {
        if (move_processor_->update()) {
            move_processor_.reset(nullptr);
            undo_stack_.push(std::move(delta_frame_));
            delta_frame_ = std::make_unique<DeltaFrame>();
        } else {
            return;
        }
    }
    if (ignore_input) {
        return;
    }
    RoomMap* room_map = room_->room_map();
    // TODO: Make a real "death" flag/state
    // Don't allow other input if player is "dead"
    if (!dynamic_cast<Player*>(room_map->view(player_->pos()))) {
        return;
    }
    for (auto p : MOVEMENT_KEYS) {
        if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
            move_processor_ = std::make_unique<MoveProcessor>(player_, room_map, p.second, delta_frame_.get());
            if (!move_processor_->try_move()) {
                move_processor_.reset(nullptr);
                return;
            }
            input_cooldown = MAX_COOLDOWN;
            Door* door = dynamic_cast<Door*>(room_map->view(player_->shifted_pos({0,0,-1})));
            // When doors are switchable, check for state too!
            if (door && door->dest() && door->state()) {
                use_door(door->dest());
            }
            return;
        }
    }
    if (glfwGetKey(window_, GLFW_KEY_X) == GLFW_PRESS) {
        player_->toggle_riding(room_map, delta_frame_.get());
        input_cooldown = MAX_COOLDOWN;
        return;
    } else if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
        MoveProcessor(player_, room_map, {0,0,0}, delta_frame_.get()).color_change_check();
        input_cooldown = MAX_COOLDOWN;
        return;
    }
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
    // This will be much more complicated when save files are a thing
    std::string path = MAPS_TEMP + name + ".map";
    if (access(path.c_str(), F_OK) == -1) {
        path = MAPS_MAIN + name + ".map";
        if (access(path.c_str(), F_OK) == -1) {
            return false;
        }
    }
    MapFileI file {path};
    auto room = std::make_unique<Room>(name);
    room->load_from_file(*objs_, file);
    // Load dynamic component!
    //room->room_map()->set_initial_state(false);
    loaded_rooms_[name] = std::move(room);
    return true;
}

void PlayingState::use_door(MapLocation* dest) {
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
    delta_frame_->push(std::make_unique<DoorMoveDelta>(this, room_, player_->pos()));
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
