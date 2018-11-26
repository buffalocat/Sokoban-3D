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
    void set_cam_pos(Point3);
    void set_cam_target(Point3);
    bool valid(Point3);
    RoomMap* room_map();

    void write_to_file(MapFileO& file, Point3 start_pos);
    void load_from_file(MapFileI& file, Point3* start_pos=nullptr);

    void draw(GraphicsManager*, Point3 cam_pos, bool ortho);

    void push_signaler(std::unique_ptr<Signaler>);

private:
    std::string name_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;

    std::vector<std::unique_ptr<Signaler>> signalers_;

    void read_objects(MapFileI& file);
    void read_camera_rects(MapFileI& file);
    void read_snake_link(MapFileI& file);
    void read_door_dest(MapFileI& file);
    void read_signaler(MapFileI& file);
};

#endif // ROOM_H
