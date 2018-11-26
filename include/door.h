#ifndef DOOR_H
#define DOOR_H

#include "common.h"
#include "switch.h"

class RoomMap;

struct MapLocation {
    Point3 pos;
    std::string name;
    MapLocation(Point3 p, std::string room_name);
};

class Door: public Switchable {
public:
    Door(Point3 pos, bool def);
    ~Door();
    ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    bool relation_check();
    void relation_serialize(MapFileO& file);
    bool can_set_state(bool state, RoomMap*);

    void set_dest(Point3, std::string);
    MapLocation* dest();

    void draw(GraphicsManager*);

private:
    std::unique_ptr<MapLocation> dest_;
};

#endif // DOOR_H
