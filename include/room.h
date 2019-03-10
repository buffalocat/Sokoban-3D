#ifndef ROOM_H
#define ROOM_H

#include <string>
#include <memory>

#include "point.h"

class GameObjectArray;
class GraphicsManager;
class RoomMap;
class Camera;
class MapFileI;
class MapFileO;
class GameObject;

class Room {
public:
    Room(const std::string& name);
    ~Room();
    std::string const name();
    void initialize(GameObjectArray& objs, int w, int h, int d);
    void set_cam_pos(Point3);
    bool valid(Point3);
    RoomMap* map();

    void write_to_file(MapFileO& file, Point3 start_pos);
    void load_from_file(GameObjectArray& objs, MapFileI& file, Point3* start_pos=nullptr);

    void draw(GraphicsManager*, Point3 cam_pos, bool ortho, bool one_layer);
    void draw(GraphicsManager*, GameObject* target, bool ortho, bool one_layer);
    void update_view(GraphicsManager*, Point3 vpos, FPoint3 rpos, bool ortho);

    void extend_by(Point3 d);
    void shift_by(Point3 d);

private:
    std::string name_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;
    Point3_S16 offset_pos_;
    // This is used exclusively for making sure doors between rooms stay accurate

    void read_objects(MapFileI& file);
    void read_camera_rects(MapFileI& file);
    void read_snake_link(MapFileI& file);
    void read_door_dest(MapFileI& file);
    void read_signaler(MapFileI& file);
    void read_walls(MapFileI& file);
    void read_player_data(MapFileI& file);
};


#endif // ROOM_H
