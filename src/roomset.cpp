/*

#include <unistd.h>

#include "room.h"

#include "editor.h"
#include "camera.h"
#include "shader.h"

#include "block.h"
#include "door.h"
#include "switch.h"

RoomSet::RoomSet(): rooms_ {} {}

Room* RoomSet::load(std::string path_base, std::string name, Room* room) {
    std::string path;
    std::ifstream file;
    if (!room) {
        path = path_base + name + ".map";
        if (access((path).c_str(), F_OK) == -1) {
            std::cout << "File \"" << path << "\" doesn't exist! Load failed." << std::endl;
            return false;
        }
        file.open(path, std::ios::in | std::ios::binary);
        room = load_from_file(file, nullptr);
        file.close();
    }
    path = path_base + name + ".mapd";
    if (access((path).c_str(), F_OK) != -1) {
        file.open(path, std::ios::in | std::ios::binary);
        load_from_file(file, room);
        file.close();
    }
    return true;
}

void RoomSet::load_from_file(std::ifstream& file, std::string name, Room* room, Point* start_pos) {
    unsigned char b[8];
    bool reading_file = true;
    RoomMap* room_map;
    Camera* camera;
    if (room) {
        room_map = room.room_map();
        camera = room.camera();
    }
    while (reading_file) {
        file.read((char *)b, 1);
        switch (static_cast<MapCode>(b[0])) {
        case MapCode::SmallDims :
            file.read((char *)b, 2);
            auto room_unique = std::make_unique<Room>(b[0], b[1]);
            room = room_unique.get();
            rooms_[name] = std::move(room_unique);
            room_map = room.room_map();
            camera = room.camera();
            break;
        case MapCode::DefaultPos :
            file.read((char *)b, 2);
            if (start_pos) {
                *start_pos = Point{b[0], b[1]};
            }
            break;
        case MapCode::Objects :
            read_objects(file, room_map, this);
            break;
        case MapCode::CameraRect :
            read_camera_rects(file, camera);
            break;
        case MapCode::SnakeLink :
            read_snake_link(file, room_map);
            break;
        case MapCode::DoorDest :
            read_door_dest(file, room_map);
            break;
        case MapCode::End :
            reading_file = false;
            break;
        default :
            std::cout << "unknown state code" << std::endl;
            //throw std::runtime_error("Unknown State code encountered in .map file (it's probably corrupt/an old version)");
            break;
        }
    }
}

void RoomSet::save(std::string path, Room* room) {
    file.open(path, std::ios::out | std::ios::binary);
    room->serialize(file);
    file.close();
}

const std::unordered_map<ObjCode, unsigned int, ObjCodeHash> BYTES_PER_OBJECT = {
    {ObjCode::NONE, 0},
    {ObjCode::Wall, 2},
    {ObjCode::PushBlock, 4},
    {ObjCode::SnakeBlock, 4},
    {ObjCode::Door, 2},
    {ObjCode::Player, 3},
    {ObjCode::PlayerWall, 2},
};

const std::unordered_map<CameraCode, unsigned int, CameraCodeHash> BYTES_PER_CAMERA = {
    {CameraCode::NONE, 0},
    {CameraCode::Free, 9},
    {CameraCode::Fixed, 13},
    {CameraCode::Clamped, 11},
    {CameraCode::Null, 5},
};

#define CASE_OBJCODE(CLASS)\
case ObjCode::CLASS :\
    room_map->put_quiet(std::unique_ptr<GameObject>(CLASS::deserialize(b)));\
    break;

void read_objects(std::ifstream& file, RoomMap* room_map, RoomManager* mgr) {
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
        case ObjCode::Player :
            {
                auto player_unique = std::unique_ptr<GameObject>(Player::deserialize(b));
                mgr->set_player(static_cast<Player*>(player_unique.get()));
                room_map->put_quiet(std::move(player_unique));
                break;
            }
        CASE_OBJCODE(PlayerWall)
        CASE_OBJCODE(Switch)
        CASE_OBJCODE(Gate)
        case ObjCode::NONE :
            return;
        default :
            throw std::runtime_error("Unknown Object code encountered in .map file (it's probably corrupt/an old version)");
            break;
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
            return;
        default :
            throw std::runtime_error("Unknown Camera code encountered in .map file (it's probably corrupt/an old version)");
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


//*/
