#ifndef ROOM_H
#define ROOM_H

#include "common.h"

class RoomMap;
class Camera;

class Room {
public:
    Room(std::string name, int w, int h);
    /*
    RoomMap* room_map();
    Camera* camera();
    std::string name(); //*/
    void serialize(std::ofstream& file);
    void draw(bool ortho);
    void draw_at(bool ortho, Point pos);

private:
    std::string name_;
    std::unique_ptr<RoomMap> map_;
    std::unique_ptr<Camera> camera_;
    Point default_start_pos_;
};

#endif // ROOM_H
