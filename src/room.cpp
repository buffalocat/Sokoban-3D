#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

#include "room.h"
#include "moveprocessor.h"
#include "block.h"
#include "shader.h"

const char* MAP_DIRECTORY = "maps\\";

// The "load" constructor
Room::Room(GLFWwindow* window, Shader* shader, std::string map_name):
    window_ {window},
    shader_ {shader},
    map_ {},
    camera_ {},
    undo_stack_ {std::make_unique<UndoStack>(DEFAULT_UNDO_DEPTH)},
    cooldown_ {0}
{
    load(map_name);
}

// The "default room" constructor
Room::Room(GLFWwindow* window, Shader* shader, int width, int height):
    window_ {window},
    shader_ {shader},
    map_ {},
    camera_ {},
    undo_stack_ {std::make_unique<UndoStack>(DEFAULT_UNDO_DEPTH)},
    cooldown_ {0}
{
    width = std::max(1, std::min(256, width));
    height = std::max(1, std::min(256, height));
    map_ = std::make_unique<RoomMap>(width, height);
    camera_ = std::make_unique<Camera>(map_.get());
}

// This is essentially the whole game loop
void Room::main_loop() {
    auto delta_frame = std::make_unique<DeltaFrame>();
    handle_input(delta_frame.get());
    draw();
    undo_stack_->push(std::move(delta_frame));
}

void Room::handle_input(DeltaFrame* delta_frame) {
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
    if (cooldown_ == 0) {
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
            undo_stack_->pop(map_.get());
            cooldown_ = MAX_COOLDOWN;
        }
    } else {
        --cooldown_;
    }
}

void Room::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera_->update();
    float cam_radius = camera_->get_radius();
    FPoint target_pos = camera_->get_pos();

    // NOTE: These belong in the camera class later
    float cam_incline = 0.1;
    float cam_rotation = 0.0;

    float cam_x = cos(cam_incline) * sin(cam_rotation) * cam_radius;
    float cam_y = sin(cam_incline) * cam_radius;
    float cam_z = cos(cam_incline) * cos(cam_rotation) * cam_radius;

    view = glm::lookAt(glm::vec3(cam_x + target_pos.x, cam_y, cam_z + target_pos.y),
                       glm::vec3(target_pos.x, 0.0f, target_pos.y),
                       glm::vec3(0.0f, 1.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.5, 0.0, 0.5));
    projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f);

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

void Room::save(std::string map_name) {
    std::string file_name = MAP_DIRECTORY + map_name;
    std::ofstream file;
    file.open(file_name, std::ios::out | std::ios::binary);

    file << static_cast<unsigned char>(State::SmallDims);
    file << static_cast<unsigned char>(map_->width());
    file << static_cast<unsigned char>(map_->height());

    file << static_cast<unsigned char>(State::Objects);
    map_->serialize(file);

    file << static_cast<unsigned char>(State::End);
    file.close();
}

void Room::load(std::string map_name) {
    std::string file_name = MAP_DIRECTORY + map_name;
    std::ifstream file;
    file.open(file_name, std::ios::in | std::ios::binary);
    unsigned char buffer[8];
    bool reading_file = true;
    int counter = 0;
    while (reading_file) {
        file.read((char *)buffer, 1);
        if (counter++ > 100) {
            break;
        }
        switch (static_cast<State>(buffer[0])) {
        case State::SmallDims : {
            file.read((char *)buffer, 2);
            map_ = std::make_unique<RoomMap>(buffer[0], buffer[1]);
            camera_ = std::make_unique<Camera>(map_.get());
            break;
        }
        case State::BigDims : {
            // This will be used later, mayber
            break;
        }
        case State::Objects : {
            read_objects(file);
            break;
        }
        case State::End : {
            reading_file = false;
            break;
        }
        default : {
            reading_file = false;
            break;
        }
        }
    }
    file.close();
}

void Room::read_objects(std::ifstream& file) {
    unsigned char buffer[8];
    file.read(reinterpret_cast<char *>(buffer), 1);
    ObjCode code = static_cast<ObjCode>(buffer[0]);
    if (code == ObjCode::NONE) {
        return;
    }
    file.read((char *)buffer, BYTES_PER_OBJECT.at(code));
    int px = buffer[0];
    int py = buffer[1];
    switch(code) {
    case ObjCode::NONE :
        break;
    case ObjCode::Wall : {
        map_->put_quiet(std::make_unique<Wall>(px, py));
        break;
    }
    case ObjCode::PushBlock : {
        bool car = (buffer[2] >> 7) == 1;
        StickyLevel sticky = static_cast<StickyLevel>(buffer[2] & 3);
        map_->put_quiet(std::make_unique<PushBlock>(px, py, car, sticky));
        break;
    }
    case ObjCode::SnakeBlock : {
        bool car = (buffer[2] >> 7) == 1;
        int ends = (buffer[2] & 1) + 1;
        map_->put_quiet(std::make_unique<SnakeBlock>(px, py, car, ends));
        for (int i = 0; i < 4; ++i) {
            if ((buffer[2] >> i) & 2) { // Effectively, shift right by i+1
                Point d = DIRECTIONS[i];
                // Check whether there's an object adjacent to this one AND whether it's a SnakeBlock
                SnakeBlock* adj = dynamic_cast<SnakeBlock*>(map_->view(Point{px + d.x, py + d.y}, Layer::Solid));
                if (adj) {
                    adj->add_link(static_cast<Block*>(map_->view(Point{buffer[0], buffer[1]}, Layer::Solid)), nullptr);
                }
            }
        }
        break;
    }
    default :
        break;
    }
}
