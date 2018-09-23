#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"

#include <dear/imgui.h>

#pragma GCC diagnostic pop

#include "room.h"
#include "moveprocessor.h"
#include "block.h"
#include "shader.h"
#include "editor.h"

const char* MAP_DIRECTORY = "maps\\";

// The "load" constructor
Room::Room(GLFWwindow* window, Shader* shader, std::string map_name):
    window_ {window},
    shader_ {shader},
    editor_ {},
    map_ {},
    camera_ {},
    undo_stack_ {},
    cooldown_ {0},
    player_ {}
{
    load(map_name);
}

// The "default room" constructor
Room::Room(GLFWwindow* window, Shader* shader, int width, int height):
    window_ {window},
    shader_ {shader},
    editor_ {},
    map_ {},
    camera_ {},
    undo_stack_ {},
    cooldown_ {0},
    player_ {}
{
    width = std::max(1, std::min(256, width));
    height = std::max(1, std::min(256, height));
    map_.reset(new RoomMap(width, height));
    camera_.reset(new Camera(map_.get()));
    undo_stack_.reset(new UndoStack(DEFAULT_UNDO_DEPTH));
}

void Room::set_editor(Editor* editor) {
    editor_ = editor;
}

// This is essentially the whole game loop
void Room::main_loop(bool& editor_mode) {
    auto delta_frame = std::make_unique<DeltaFrame>();
    if (cooldown_ == 0 && glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS
        && glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        editor_mode = !editor_mode;
        cooldown_ = MAX_COOLDOWN;
    }
    if (editor_mode) {
        handle_input_editor_mode();
        draw_editor_mode();
    } else {
        handle_input(delta_frame.get());
        draw(false);
    }
    undo_stack_->push(std::move(delta_frame));
}

void Room::handle_input(DeltaFrame* delta_frame) {
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
                MoveProcessor(map_.get(), p.second).try_move(delta_frame);
                camera_->set_target(player_->pos());
                //std::cout << "Move took " << GetCounter() << std::endl;
                cooldown_ = MAX_COOLDOWN;
                break;
            }
        }
    }
    // We can't move and undo on the same frame, so we check again
    if (cooldown_ == 0) {
        if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
            if (undo_stack_->pop(map_.get())) {
                if (player_) {
                    camera_->set_current_pos(player_->pos());
                }
                cooldown_ = MAX_COOLDOWN;
            }
        }
    } else {
        --cooldown_;
    }
}

void Room::handle_input_editor_mode() {
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
                editor_->clamp_pos(map_->width(), map_->height());
                camera_->set_current_pos(editor_->pos());
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

void Room::draw(bool editor_mode) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_->update();
    float cam_radius = camera_->get_radius();
    FPoint target_pos = camera_->get_pos();

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

    map_->draw(shader_);

    // Draw the floor
    model = glm::translate(glm::mat4(), glm::vec3(-0.5, -0.1, -0.5));
    model = glm::scale(model, glm::vec3(map_->width(), 0.1, map_->height()));
    model = glm::translate(model, glm::vec3(0.5, -0.1, 0.5));
    shader_->setMat4("model", model);

    shader_->setVec4("color", YELLOW);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
}

void Room::draw_editor_mode() {
    // Draw the usual things, but in Ortho mode
    draw(true);
    // Maybe draw other indicators over the map
}

//NOTE: obj guaranteed to be valid
void Room::create_obj(std::unique_ptr<GameObject> obj) {
    if (!map_->view(obj->pos(), Layer::Solid)) {
        Block* block = dynamic_cast<Block*>(obj.get());
        map_->put_quiet(std::move(obj));
        if (block) {
            block->check_add_local_links(map_.get(), nullptr);
        }
    }
}

void Room::delete_obj(Point pos) {
    GameObject* obj = map_->view(pos, Layer::Solid);
    if (obj) {
        obj->cleanup(nullptr);
        auto sb = dynamic_cast<SnakeBlock*>(obj);
        map_->take_quiet(obj);
        if (sb) {
            for (auto& d : DIRECTIONS) {
                auto snake = dynamic_cast<SnakeBlock*>(map_->view(Point{pos.x + d.x, pos.y + d.y}, Layer::Solid));
                if (snake) {
                    snake->check_add_local_links(map_.get(), nullptr);
                }
            }
        }
    }
}



Point Room::get_pos_from_mouse() {
    double xpos, ypos;
    glfwGetCursorPos(window_, &xpos, &ypos);
    FPoint cam_pos = camera_->get_pos();
    if (xpos >= 0 && xpos < SCREEN_WIDTH && ypos >= 0 && ypos < SCREEN_HEIGHT) {
        int x = ((int)xpos + MESH_SIZE*cam_pos.x - (SCREEN_WIDTH - MESH_SIZE) / 2) / MESH_SIZE;
        int y = ((int)ypos + MESH_SIZE*cam_pos.y - (SCREEN_HEIGHT - MESH_SIZE) / 2) / MESH_SIZE;
        return Point{x, y};
    }
    return Point{-1, -1};
}

bool Room::valid(Point pos) {
    return map_->valid(pos);
}

void Room::save(std::string map_name, bool overwrite) {
    std::string file_name = MAP_DIRECTORY + map_name + ".map";
    std::ofstream file;
    if (access((file_name).c_str(), F_OK) != -1 && !overwrite) {
        std::cout << "File \"" << file_name << "\" already exists! Save failed." << std::endl;
        return;
    }
    file.open(file_name, std::ios::out | std::ios::binary);

    file << static_cast<unsigned char>(State::SmallDims);
    file << static_cast<unsigned char>(map_->width());
    file << static_cast<unsigned char>(map_->height());

    map_->serialize(file);

    file << static_cast<unsigned char>(State::End);
    file.close();
    std::cout <<  file_name << " saved." << std::endl;
}

void Room::load(std::string map_name) {
    if (map_name.size() == 0) {
        return;
    }
    std::string file_name = MAP_DIRECTORY + map_name + ".map";
    std::ifstream file;
    if (access((file_name).c_str(), F_OK) == -1) {
        std::cout << "File \"" << file_name << "\" doesn't exist! Load failed." << std::endl;
        return;
    }
    file.open(file_name, std::ios::in | std::ios::binary);
    unsigned char buffer[8];
    bool reading_file = true;
    while (reading_file) {
        file.read((char *)buffer, 1);
        switch (static_cast<State>(buffer[0])) {
        case State::SmallDims :
            file.read((char *)buffer, 2);
            map_.reset(new RoomMap(buffer[0], buffer[1]));
            camera_.reset(new Camera(map_.get()));
            undo_stack_.reset(new UndoStack(DEFAULT_UNDO_DEPTH));
            break;
        case State::BigDims :
            // This will be used later, maybe
            break;
        case State::Objects :
            read_objects(file);
            break;
        case State::CameraRect :
            read_camera_rects(file);
            break;
        case State::SnakeLink :
            read_snake_link(file);
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
    map_->set_initial_state();
    player_ = map_->get_mover();
    if (player_) {
        camera_->set_current_pos(player_->pos());
    } else {
        camera_->set_current_pos(Point{0,0});
    }
    std::cout << map_name << " loaded." << std::endl;
}

void Room::read_objects(std::ifstream& file) {
    unsigned char buffer[8];
    while (true) {
        file.read(reinterpret_cast<char *>(buffer), 1);
        ObjCode code = static_cast<ObjCode>(buffer[0]);
        file.read((char *)buffer, BYTES_PER_OBJECT.at(code));
        switch (code) {
        case ObjCode::Wall :
            map_->put_quiet(std::unique_ptr<GameObject>(Wall::deserialize(buffer)));
            break;
        case ObjCode::PushBlock :
            map_->put_quiet(std::unique_ptr<GameObject>(PushBlock::deserialize(buffer)));
            break;
        case ObjCode::SnakeBlock :
            map_->put_quiet(std::unique_ptr<GameObject>(SnakeBlock::deserialize(buffer)));
            break;
        case ObjCode::NONE :
        default :
            return;
        }
    }
}

void Room::read_camera_rects(std::ifstream& file) {

}

void Room::read_snake_link(std::ifstream& file) {
    unsigned char buffer[3];
    file.read(reinterpret_cast<char *>(buffer), 3);
    SnakeBlock* sb = static_cast<SnakeBlock*>(map_->view(Point{buffer[0], buffer[1]}, Layer::Solid));
    // Linked right
    if (buffer[2] & 1) {
        sb->add_link(static_cast<SnakeBlock*>(map_->view(Point{buffer[0]+1, buffer[1]}, Layer::Solid)), nullptr);
    }
    // Linked down
    if (buffer[2] & 2) {
        sb->add_link(static_cast<SnakeBlock*>(map_->view(Point{buffer[0], buffer[1]+1}, Layer::Solid)), nullptr);
    }
}
