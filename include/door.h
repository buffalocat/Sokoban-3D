#ifndef DOOR_H
#define DOOR_H

#include "common.h"
#include "switch.h"

class RoomMap;

struct MapLocation {
    Point pos;
    std::string name;
    MapLocation(Point p, std::string room_name);
};

class Door: public Switchable {
public:
    Door(int x, int y, bool def);
    ~Door();
    ObjCode obj_code();
    Layer layer();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    bool relation_check();
    void relation_serialize(std::ofstream& file);
    bool can_set_state(bool state, RoomMap*);

    void set_dest(Point, std::string);
    MapLocation* dest();

    void draw(GraphicsManager*);

private:
    std::unique_ptr<MapLocation> dest_;
};

#endif // DOOR_H
