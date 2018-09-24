#ifndef DOOR_H
#define DOOR_H

#include "common.h"
#include "gameobject.h"

struct MapLocation {
    Point pos;
    std::string map_name;
    MapLocation(Point p, std::string name);
};

class Door: public GameObject {
public:
    Door(int x, int y);
    ~Door();
    ObjCode obj_code();
    Layer layer();
    static GameObject* deserialize(unsigned char* buffer);
    bool relation_check();
    void relation_serialize(std::ofstream& file);

    void set_dest(Point, std::string);
    MapLocation* dest();

    void draw(Shader*);

private:
    std::unique_ptr<MapLocation> dest_;
};

#endif // DOOR_H
