#ifndef DOOR_H
#define DOOR_H

#include <string>
#include <memory>

#include "objectmodifier.h"
#include "switchable.h"


class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

struct MapLocation {
    Point3 pos;
    std::string name;
    MapLocation(Point3 p, std::string room_name);
};

class Door: public Switchable {
public:
    Door(GameObject* parent, bool def, bool active);
    virtual ~Door();
    Door(const Door&);

    std::string name();
    ModCode mod_code();
    void serialize(MapFileO& file);
    static void deserialize(MapFileI&, RoomMap*, GameObject*);

    bool relation_check();
    void relation_serialize(MapFileO& file);
    bool can_set_state(bool state, RoomMap*);

    void set_dest(Point3, std::string);
    MapLocation* dest();

    void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);

    void setup_on_put(RoomMap* );
    void cleanup_on_take(RoomMap*);

    void draw(GraphicsManager*, FPoint3);

    std::unique_ptr<ObjectModifier> duplicate(GameObject*, RoomMap*, DeltaFrame*);

private:
    std::unique_ptr<MapLocation> dest_;
};

#endif // DOOR_H
