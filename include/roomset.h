#ifndef ROOMSET_H
#define ROOMSET_H

#include "common.h"

class Room;

class RoomSet {
public:
    RoomSet();

protected:
    void load(std::string path_base, std::string name, Room* room=nullptr);
    void load_from_file(std::ifstream& file, std::string name, Room* room=nullptr, Point* start_pos=nullptr);
    /// Save room to a single .map file
    void save(std::string path, Room* room);
    /// Save room to a .map (static) and a .mapd (dynamic)
    //void save_split(std::string path, Room* room);
    /// Save only dynamic elements of a room to a .mapd
    //void save_dynamic(std::string path, Room* room);
    std::map<std::string, std::unique_ptr<Room>> rooms_;
};

void read_objects(std::ifstream& file, RoomMap*);
void read_camera_rects(std::ifstream& file, Camera*);
void read_snake_link(std::ifstream& file, RoomMap*);
void read_door_dest(std::ifstream& file, RoomMap*);

#endif // ROOMSET_H
