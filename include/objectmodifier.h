#ifndef OBJECTMODIFIER_H
#define OBJECTMODIFIER_H

#include <vector>

#include "common.h"
#include "point.h"

class GameObject;
class RoomMap;
class DeltaFrame;
class MoveProcessor;
class MapFileI;
class MapFileO;
class GraphicsManager;

// Base class of object modifiers such as Car, Door, Switch, and Gate
class ObjectModifier {
public:
    ObjectModifier(GameObject* parent);
    virtual ~ObjectModifier();

    virtual ModCode mod_code() = 0;
    virtual void serialize(MapFileO& file) = 0;

    GameObject* parent_;

    // TODO: make this pure virtual?
    virtual void draw(GraphicsManager*, FPoint3) {}

    Point3 pos();
    Point3 shifted_pos(Point3 d);
    Point3 pos_above(); // A convenience function often needed by Modifiers
    int color();
    bool pushable();
    bool gravitable();

    virtual void setup_on_put(RoomMap* room_map);
    virtual void cleanup_on_take(RoomMap* room_map);
    virtual void cleanup_on_destruction(RoomMap* room_map);
    virtual void setup_on_undestruction(RoomMap* room_map);

    // TODO: This should actually be pure virtual! But we don't care for now.
    virtual std::unique_ptr<ObjectModifier> duplicate();

    // Every type of Modifier can have at most one callback function for map listeners
    virtual void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*);
    virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&);

};

#endif // OBJECTMODIFIER_H
