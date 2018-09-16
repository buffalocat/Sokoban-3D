#include "loader.h"

#include <unistd.h>
#include <fstream>
#include <iostream>

#include "worldmap.h"
#include "gameobject.h"
#include "block.h"

const char* MAP_DIRECTORY = "maps\\";

enum State {
    SmallDims = 1, // Gets width and height as 1 byte integers
    BigDims = 2, // Gets width and height as 2 byte integers
    Objects = 3, // Read in all map objects
    End = 255,
};

static std::unordered_map<ObjCode, unsigned int, ObjCodeHash> BYTES_PER_OBJECT = {
    {ObjCode::NONE, 0},
    {ObjCode::Wall, 2},
    {ObjCode::PushBlock, 3},
    {ObjCode::SnakeBlock, 3},
};

void Loader::save(const WorldMap* world_map) {
    std::cout << "Enter name to save file as: (enter blank line to quit saving)" << std::endl;
    std::string file_name;
    std::getline(std::cin, file_name);
    if (file_name.empty()) {
        std::cout << "File name was empty; returning to game." << std::endl;
        return;
    }
    file_name = MAP_DIRECTORY + file_name;
    if (access(file_name.c_str(), F_OK) != -1) {
        std::cout << "File already exists! Save failed." << std::endl;
        return;
    } else {
        std::cout << "That file name is ok!" << std::endl;
    }
    std::ofstream file;
    file.open(file_name, std::ios::out | std::ios::binary);
    std::cout << "Opened File" << std::endl;
    bool big_map = world_map->width() > 255 || world_map->height() > 255;
    if (big_map) {
        ; // Not allowing this yet
    } else {
        file << static_cast<unsigned char>(State::SmallDims);
        file << static_cast<unsigned char>(world_map->width());
        file << static_cast<unsigned char>(world_map->height());
    }
    file << static_cast<unsigned char>(State::Objects);
    world_map->serialize(file);
    file << static_cast<unsigned char>(State::End);
    file.close();
    std::cout << "Exiting save dialog" << std::endl;
}

WorldMap* Loader::load() {
    std::cout << "Enter name of file to load from: (enter blank line to quit loading)" << std::endl;
    std::string file_name;
    std::getline(std::cin, file_name);
    if (file_name.empty()) {
        std::cout << "File name was empty; returning to game." << std::endl;
        return nullptr;
    }
    file_name = MAP_DIRECTORY + file_name;
    if (access(file_name.c_str(), F_OK) == -1) {
        std::cout << "File doesn't exist! Load failed." << std::endl;
        return nullptr;
    }
    std::ifstream file;
    file.open(file_name, std::ios::in | std::ios::binary);
    unsigned char buffer[8];
    bool reading_file = true;
    WorldMap* world_map = nullptr;
    int counter = 0;
    while (reading_file) {
        file.read((char *)buffer, 1);
        if (counter++ > 100) {
            break;
        }
        switch (static_cast<State>(buffer[0])) {
        case State::SmallDims : {
            file.read((char *)buffer, 2);
            world_map = new WorldMap(buffer[0], buffer[1]);
            break;
        }
        case State::BigDims : {
            // This will be used later, mayber
            break;
        }
        case State::Objects : {
            bool reading_objects = true;
            while (reading_objects) {
                file.read(reinterpret_cast<char *>(buffer), 1);
                ObjCode code = static_cast<ObjCode>(buffer[0]);
                if (code == ObjCode::NONE) {
                    reading_objects = false;
                    break;
                }
                unsigned int bytes = BYTES_PER_OBJECT[code];
                file.read((char *)buffer, bytes);
                int px = buffer[0];
                int py = buffer[1];
                switch(code) {
                case ObjCode::NONE : break;
                case ObjCode::Wall : {
                    world_map->put_quiet(std::make_unique<Wall>(px, py));
                    break;
                }
                case ObjCode::PushBlock : {
                    bool car = (buffer[2] >> 7) == 1;
                    StickyLevel sticky = static_cast<StickyLevel>(buffer[2] & 3);
                    world_map->put_quiet(std::make_unique<PushBlock>(px, py, car, sticky));
                    break;
                }
                case ObjCode::SnakeBlock : {
                    bool car = (buffer[2] >> 7) == 1;
                    int ends = (buffer[2] & 1) + 1;
                    world_map->put_quiet(std::make_unique<SnakeBlock>(px, py, car, ends));
                    for (int i = 0; i < 4; ++i) {
                        if ((buffer[2] >> i) & 2) { // Effectively, shift right by i+1
                            Point d = DIRECTIONS[i];
                            // Check whether there's an object adjacent to this one AND whether it's a SnakeBlock
                            SnakeBlock* adj = dynamic_cast<SnakeBlock*>(world_map->view(Point{px + d.x, py + d.y}, Layer::Solid));
                            if (adj) {
                                adj->add_link(static_cast<Block*>(world_map->view(Point{buffer[0], buffer[1]}, Layer::Solid)), nullptr);
                            }
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
                }
            }
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
    return world_map;
}
