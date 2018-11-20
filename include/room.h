#ifndef ROOM_H
#define ROOM_H

#include "common.h"
#include "roommap.h"
#include "camera.h"
#include "switch.h"

class GraphicsManager;

class Room {
public:
    Room(std::string name);
    Room(std::string name, int w, int h);
    ~Room();
    std::string const name();
    void initialize(int w, int h);
    void set_cam_pos(Point);
    void set_cam_target(Point);
    bool valid(Point pos);
    RoomMap* room_map();

    void write_to_file(std::ofstream& file, Point start_pos);
    void load_from_file(std::ifstream& file, Point* start_pos=nullptr);

    void draw(GraphicsManager*, Point cam_pos, bool ortho);

    void push_signaler(std::unique_ptr<Signaler>);

private:
    std::string name_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;

    std::vector<std::unique_ptr<Signaler>> signalers_;

    void read_objects(std::ifstream& file);
    void read_camera_rects(std::ifstream& file);
    void read_snake_link(std::ifstream& file);
    void read_door_dest(std::ifstream& file);
    void read_signaler(std::ifstream& file);
};

#endif // ROOM_H
