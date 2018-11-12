#ifndef ROOM_H
#define ROOM_H

#include "common.h"

class RoomMap;
class Camera;
class GraphicsManager;

class Room {
public:
    Room(std::string name);
    Room(std::string name, int w, int h);
    ~Room() = default;
    std::string const name();
    void initialize(int w, int h);
    void set_cam_pos(Point);
    void set_cam_target(Point);
    bool valid(Point pos);
    RoomMap* room_map();

    void write_to_file(std::ofstream& file, Point start_pos);
    void load_from_file(std::ifstream& file, Point* start_pos=nullptr);

    void draw(GraphicsManager*, Point cam_pos, bool ortho);

private:
    std::string name_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;

    void read_objects(std::ifstream& file);
    void read_camera_rects(std::ifstream& file);
    void read_snake_link(std::ifstream& file);
    void read_door_dest(std::ifstream& file);
};

#endif // ROOM_H
