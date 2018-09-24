#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include <dear/imgui.h>

#pragma GCC diagnostic pop

#include "room.h"

#include "editor.h"
#include "camera.h"
#include "shader.h"

#include "moveprocessor.h"
#include "block.h"
#include "door.h"

Room::Room(std::string name, std::unique_ptr<RoomMap> room_map, std::unique_ptr<Camera> camera):
    name_ {name},
    map_ {std::move(room_map)},
    camera_ {std::move(camera)},
    doors_ {} {}

Room::Room(int w, int h): name_ {""}, map_ {}, camera_ {}, doors_ {} {
    map_ = std::make_unique<RoomMap>(w, h);
    camera_ = std::make_unique<Camera>(map_.get());
}

RoomMap* Room::room_map() {
    return map_.get();
}

Camera* Room::camera() {
    return camera_.get();
}

std::string Room::name() {
    return name_;
}

RoomManager::RoomManager(GLFWwindow* window, Shader* shader):
    window_ {window},
    shader_ {shader},
    editor_ {},
    undo_stack_ {UndoStack(MAX_UNDO_DEPTH)},
    cooldown_ {0},
    player_ {},
    rooms_ {},
    cur_room_ {},
    cur_map_ {},
    cur_camera_ {} {}

void RoomManager::set_editor(Editor* editor) {
    editor_ = editor;
}

void RoomManager::set_cur_room(Room* room) {
    cur_room_ = room;
    cur_map_ = room->room_map();
    cur_camera_ = room->camera();
}

RoomMap* RoomManager::room_map() {
    return cur_map_;
}

Camera* RoomManager::camera() {
    return cur_camera_;
}

// This is essentially the whole game loop
void RoomManager::main_loop(bool& editor_mode) {
    auto delta_frame = std::make_unique<DeltaFrame>();
    if (cooldown_ == 0 && glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS
        && glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        editor_mode = !editor_mode;
        undo_stack_.reset();
        cooldown_ = MAX_COOLDOWN;
    }
    if (editor_mode) {
        handle_input_editor_mode();
        draw_editor_mode();
    } else {
        handle_input(delta_frame.get());
        draw(false);
    }
    undo_stack_.push(std::move(delta_frame));
}

void RoomManager::handle_input(DeltaFrame* delta_frame) {
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
    if (cooldown_ == 0 && player_) {
        for (auto p : MOVEMENT_KEYS) {
            // For starters, we'll use a naive input processing method.
            // In particular, the precedence of keys is arbitrary
            // and there is no buffering.
            if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
                //StartCounter();
                MoveProcessor(cur_map_, p.second).try_move(delta_frame);
                Door* door = static_cast<Door*>(cur_map_->view(player_->pos(), ObjCode::Door));
                if (door && door->dest()) {
                    use_door(door->dest(), delta_frame);
                }
                cur_camera_->set_target(player_->pos());
                //std::cout << "Move took " << GetCounter() << std::endl;
                cooldown_ = MAX_COOLDOWN;
                break;
            }
        }
    }
    // We can't move and undo on the same frame, so we check again
    if (cooldown_ == 0) {
        if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
            if (undo_stack_.pop()) {
                if (player_) {
                    cur_camera_->set_current_pos(player_->pos());
                }
                cooldown_ = MAX_COOLDOWN;
            }
        }
    } else {
        --cooldown_;
    }
}

void RoomManager::handle_input_editor_mode() {
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
    if (cooldown_ == 0 && !editor_->want_capture_keyboard()) {
        for (auto p : MOVEMENT_KEYS) {
            if (glfwGetKey(window_, p.first) == GLFW_PRESS) {
                if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                    Point d = {FAST_MAP_MOVE * p.second.x, FAST_MAP_MOVE * p.second.y};
                    editor_->shift_pos(d);
                } else {
                    editor_->shift_pos(p.second);
                }
                editor_->clamp_pos(cur_map_->width(), cur_map_->height());
                cur_camera_->set_current_pos(editor_->pos());
                cooldown_ = MAX_COOLDOWN;
                break;
            }
        }
    }
    editor_->handle_input();
    if (cooldown_) {
        --cooldown_;
    }
}

void RoomManager::draw(bool editor_mode) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cur_camera_->update();
    float cam_radius = cur_camera_->get_radius();
    FPoint target_pos = cur_camera_->get_pos();

    // NOTE: These belong in the camera class later
    float cam_incline = 0.1;
    float cam_rotation = 0.0;

    float cam_x = sin(cam_incline) * sin(cam_rotation) * cam_radius;
    float cam_y = cos(cam_incline) * cam_radius;
    float cam_z = sin(cam_incline) * cos(cam_rotation) * cam_radius;

    if (editor_mode) {
        view = glm::lookAt(glm::vec3(target_pos.x, 2.0f, target_pos.y),
                           glm::vec3(target_pos.x, 0.0f, target_pos.y),
                           glm::vec3(0.0f, 0.0f, -1.0f));
        projection = glm::ortho(-ORTHO_WIDTH/2.0f, ORTHO_WIDTH/2.0f, -ORTHO_HEIGHT/2.0f, ORTHO_HEIGHT/2.0f, 0.0f, 3.0f);
    } else {
        view = glm::lookAt(glm::vec3(cam_x + target_pos.x, cam_y, cam_z + target_pos.y),
                           glm::vec3(target_pos.x, 0.0f, target_pos.y),
                           glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.5, 0.0, 0.5));
        projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);
    }

    shader_->setMat4("view", view);
    shader_->setMat4("projection", projection);

    cur_map_->draw(shader_);

    // Draw the floor
    model = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.1, -0.5));
    model = glm::scale(model, glm::vec3(cur_map_->width(), 0.1, cur_map_->height()));
    model = glm::translate(model, glm::vec3(0.5, -0.1, 0.5));
    shader_->setMat4("model", model);

    shader_->setVec4("color", YELLOW);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void RoomManager::draw_editor_mode() {
    // Draw the usual things, but in Ortho mode
    draw(true);
    // Maybe draw other indicators over the map
}

//NOTE: obj guaranteed to be valid
void RoomManager::create_obj(std::unique_ptr<GameObject> obj) {
    if (!cur_map_->view(obj->pos(), Layer::Solid)) {
        Block* block = dynamic_cast<Block*>(obj.get());
        cur_map_->put_quiet(std::move(obj));
        if (block) {
            block->check_add_local_links(cur_map_, nullptr);
        }
    }
}

void RoomManager::delete_obj(Point pos) {
    GameObject* obj = cur_map_->view(pos, Layer::Solid);
    if (obj) {
        obj->cleanup(nullptr);
        auto sb = dynamic_cast<SnakeBlock*>(obj);
        cur_map_->take_quiet(obj);
        if (sb) {
            for (auto& d : DIRECTIONS) {
                auto snake = dynamic_cast<SnakeBlock*>(cur_map_->view(Point{pos.x + d.x, pos.y + d.y}, Layer::Solid));
                if (snake) {
                    snake->check_add_local_links(cur_map_, nullptr);
                }
            }
        }
    }
}

void RoomManager::make_door(Point pos, Point dest, std::string room_name) {
    Door* door = static_cast<Door*>(cur_map_->view(pos, ObjCode::Door));
    if (door) {
        door->set_dest(dest, room_name);
    }
}

Point RoomManager::get_pos_from_mouse() {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    FPoint cam_pos = cur_camera_->get_pos();
    if (xpos >= 0 && xpos < SCREEN_WIDTH && ypos >= 0 && ypos < SCREEN_HEIGHT) {
        int x = ((int)xpos + MESH_SIZE*cam_pos.x - (SCREEN_WIDTH - MESH_SIZE) / 2) / MESH_SIZE;
        int y = ((int)ypos + MESH_SIZE*cam_pos.y - (SCREEN_HEIGHT - MESH_SIZE) / 2) / MESH_SIZE;
        return Point{x, y};
    }
    return Point{-1, -1};
}

bool RoomManager::valid(Point pos) {
    return cur_map_->valid(pos);
}

void Room::serialize(std::ofstream& file) {
    // Write potential load failure conditions first
    if (!map_->get_mover()) {
        file << static_cast<unsigned char>(State::NoPlayer);
    }
    for (Point p : doors_) {
        if (map_->view(p, Layer::Solid)) {
            file << static_cast<unsigned char>(State::BlockedDoor);
            file << static_cast<unsigned char>(p.x);
            file << static_cast<unsigned char>(p.y);
        }
    }

    // Ordinary Map Data past here
    file << static_cast<unsigned char>(State::SmallDims);
    file << static_cast<unsigned char>(map_->width());
    file << static_cast<unsigned char>(map_->height());

    map_->serialize(file);

    camera_->serialize(file);

    file << static_cast<unsigned char>(State::End);
}

const char* BASE_MAP_DIRECTORY = "maps\\";

//const char* TEMP_MAP_DIRECTORY = "maps\\temp\\";

void Room::save() {
    std::string file_name = BASE_MAP_DIRECTORY + name_ + ".map";
    std::ofstream file;
    file.open(file_name, std::ios::out | std::ios::binary);
    serialize(file);
    file.close();
}

void RoomManager::save(std::string map_name, bool overwrite) {
    std::string file_name = BASE_MAP_DIRECTORY + map_name + ".map";
    std::ofstream file;
    if (access((file_name).c_str(), F_OK) != -1 && !overwrite) {
        std::cout << "File \"" << file_name << "\" already exists! Save failed." << std::endl;
        return;
    }
    file.open(file_name, std::ios::out | std::ios::binary);
    cur_room_->serialize(file);
    file.close();
    std::cout <<  file_name << " saved." << std::endl;
}

std::unique_ptr<Room> RoomManager::load(std::string map_name, bool edit_mode, Point start) {
    if (map_name.size() == 0) {
        std::cout << "Tried to load map file with empty name! Load failed." << std::endl;
        return nullptr;
    }
    std::string file_name = BASE_MAP_DIRECTORY + map_name + ".map";
    std::ifstream file;
    if (access((file_name).c_str(), F_OK) == -1) {
        std::cout << "File \"" << file_name << "\" doesn't exist! Load failed." << std::endl;
        return nullptr;
    }
    file.open(file_name, std::ios::in | std::ios::binary);
    unsigned char b[8];
    bool reading_file = true;
    std::unique_ptr<RoomMap> room_map_unique;
    RoomMap* room_map;
    std::unique_ptr<Camera> camera_unique;
    Camera* camera;
    std::unique_ptr<Room> room;
    while (reading_file) {
        file.read((char *)b, 1);
        //NOTE: We can simplify this switch with macros if we use camelCase for these methods...?
        switch (static_cast<State>(b[0])) {
        // Failure flags will always come first in the .map
        // If there's no player, and we didn't bring one, and we're actually playing, load fails
        case State::NoPlayer :
            if (start.x == -1 && !edit_mode) {
                std::cout << "No player in the map! Load failed." << std::endl;
                return nullptr;
            }
            break;
        case State::BlockedDoor :
            file.read((char *)b, 2);
            if (b[0] == start.x && b[1] == start.y) {
                return nullptr;
            }
        case State::SmallDims :
            file.read((char *)b, 2);
            room_map_unique = std::make_unique<RoomMap>(b[0], b[1]);
            room_map = room_map_unique.get();
            camera_unique = std::make_unique<Camera>(room_map);
            camera = camera_unique.get();
            room = std::make_unique<Room>(map_name, std::move(room_map_unique), std::move(camera_unique));
            break;
        case State::BigDims :
            // This will be used later, maybe
            break;
        case State::Objects :
            read_objects(file, room_map);
            break;
        case State::CameraRect :
            read_camera_rects(file, camera);
            break;
        case State::SnakeLink :
            read_snake_link(file, room_map);
            break;
        case State::DoorDest :
            read_door_dest(file, room_map);
            break;
        case State::End :
            reading_file = false;
            break;
        default :
            std::cout << "Read a bad State:: code!!" << std::endl;
            reading_file = false;
            break;
        }
    }
    file.close();
    // Undoing loses meaning if we didn't get here via a door move (or something similar)
    if (start.x == -1) {
        undo_stack_.reset();
    }
    return room;
}

void RoomManager::init_load(std::string map_name) {
    std::unique_ptr<Room> room = load(map_name, true);
    if (!room) {
        return;
    }
    //rooms_.clear();
    set_cur_room(room.get());
    rooms_[room->name()] = std::move(room);
    cur_map_->set_initial_state();
    player_ = cur_map_->get_mover();
    if (player_) {
        cur_camera_->set_current_pos(player_->pos());
    } else {
        cur_camera_->set_current_pos(Point{0,0});
    }
}

void RoomManager::init_make(int w, int h) {
    std::unique_ptr<Room> room = std::make_unique<Room>(w, h);
    //rooms_.clear();
    undo_stack_.reset();
    set_cur_room(room.get());
    rooms_[room->name()] = std::move(room);
    auto player_unique = std::make_unique<PushBlock>(0,0,true,StickyLevel::None);
    player_ = player_unique.get();
    cur_map_->put_quiet(std::move(player_unique));
    cur_map_->set_initial_state();
    cur_camera_->set_current_pos(player_->pos());
}

void RoomManager::use_door(MapLocation* dest, DeltaFrame* delta_frame) {
    Room* room;
    // Use the cached version if available (this is NECESSARY for "identity" reasons)
    if (rooms_.count(dest->map_name)) {
        room = rooms_[dest->map_name].get();
        // Check if there's anything covering the door!
        if (room->room_map()->view(dest->pos, Layer::Solid)) {
            return;
        }
    } else {
        std::unique_ptr<Room> loaded_room = load(dest->map_name, false, dest->pos);
        room = loaded_room.get();
        if (!room) {
            return;
        }
        rooms_[dest->map_name] = std::move(loaded_room);
    }
    // Clean up the player however necessary BEFORE moving through the door
    player_->remove_all_links(delta_frame);
    delta_frame->push(std::make_unique<DoorMoveDelta>(this, cur_room_, player_->pos(), player_));
    auto player_unique = cur_map_->take_quiet(player_);
    player_->set_pos(dest->pos);
    // I'm not exactly sure what we want to do with saving, but this isn't it
    //cur_room_->save();
    set_cur_room(room);
    cur_map_->put_quiet(std::move(player_unique));
    player_->check_add_local_links(cur_map_, delta_frame);
    cur_map_->set_initial_state();
    cur_camera_->set_current_pos(player_->pos());
}

const std::unordered_map<ObjCode, unsigned int, ObjCodeHash> BYTES_PER_OBJECT = {
    {ObjCode::NONE, 0},
    {ObjCode::Wall, 2},
    {ObjCode::PushBlock, 3},
    {ObjCode::SnakeBlock, 3},
    {ObjCode::Door, 2},
};

const std::unordered_map<CameraCode, unsigned int, CameraCodeHash> BYTES_PER_CAMERA = {
    {CameraCode::NONE, 0},
    {CameraCode::Free, 7},
    {CameraCode::Fixed, 11},
    {CameraCode::Clamped, 9},
    {CameraCode::Null, 5},
};

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS :\
    room_map->put_quiet(std::unique_ptr<GameObject>(CLASS::deserialize(b)));\
    break;

void read_objects(std::ifstream& file, RoomMap* room_map) {
    unsigned char b[8];
    while (true) {
        file.read(reinterpret_cast<char *>(b), 1);
        ObjCode code = static_cast<ObjCode>(b[0]);
        file.read((char *)b, BYTES_PER_OBJECT.at(code));
        switch (code) {
        CASE_OBJCODE(Wall)
        CASE_OBJCODE(PushBlock)
        CASE_OBJCODE(SnakeBlock)
        CASE_OBJCODE(Door)
        case ObjCode::NONE :
        default :
            return;
        }
    }
}

#undef CASE_OBJCODE

#define CASE_CAMCODE(CLASS)\
case CameraCode::CLASS :\
    camera->push_context(std::unique_ptr<CameraContext>(CLASS ## CameraContext::deserialize(b)));\
    break;

void read_camera_rects(std::ifstream& file, Camera* camera) {
    unsigned char b[16];
    while (true) {
        file.read(reinterpret_cast<char *>(b), 1);
        CameraCode code = static_cast<CameraCode>(b[0]);
        file.read((char *)b, BYTES_PER_CAMERA.at(code));
        switch (code) {
        CASE_CAMCODE(Free)
        CASE_CAMCODE(Fixed)
        CASE_CAMCODE(Clamped)
        CASE_CAMCODE(Null)
        case CameraCode::NONE :
        default :
            return;
        }
    }
}

#undef CASE_CAMCODE

void read_snake_link(std::ifstream& file, RoomMap* room_map) {
    unsigned char b[3];
    file.read((char *)b, 3);
    SnakeBlock* sb = static_cast<SnakeBlock*>(room_map->view(Point{b[0], b[1]}, Layer::Solid));
    // Linked right
    if (b[2] & 1) {
        sb->add_link(static_cast<SnakeBlock*>(room_map->view(Point{b[0]+1, b[1]}, Layer::Solid)), nullptr);
    }
    // Linked down
    if (b[2] & 2) {
        sb->add_link(static_cast<SnakeBlock*>(room_map->view(Point{b[0], b[1]+1}, Layer::Solid)), nullptr);
    }
}

void read_door_dest(std::ifstream& file, RoomMap* room_map) {
    unsigned char b[5];
    file.read((char *)b, 5);
    auto door = static_cast<Door*>(room_map->view(Point{b[0],b[1]}, ObjCode::Door));
    char name[256];
    file.read(name, b[4]);
    door->set_dest(Point{b[2],b[3]}, std::string(name, b[4]));
}
