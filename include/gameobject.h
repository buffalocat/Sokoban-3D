#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "common.h"

class DeltaFrame;
class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

// The GameObject class will really act like the base class
// for Solid objects, until some non-solid objects exist too.
/** Abstract class for all objects that get placed on the map
 */
class GameObject {
public:
    virtual ~GameObject();
    virtual ObjCode obj_code() = 0;

    virtual void serialize(MapFileO& file);
    virtual bool relation_check();
    virtual void relation_serialize(MapFileO& file);

    Point3 pos();
    Point2 posh();
    int z();

    Point3 shifted_pos(Point3 d);
    void set_pos(Point3 p);
    void shift_pos(Point3 d);

    virtual void draw(GraphicsManager*, Point3 p);

    virtual void setup_on_put(RoomMap*);
    virtual void cleanup_on_take(RoomMap*);

    virtual void setup_on_undestruction(RoomMap*);
    virtual void cleanup_on_destruction(RoomMap*);

    virtual bool pushable();
    virtual bool gravitable();

    Point3 pos_;
    int id_;

protected:
    GameObject(Point3 pos);
};

/** An immovable, static obstacle
 */
class Wall: public GameObject {
public:
    Wall();
    ~Wall();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    void draw(GraphicsManager*, Point3 p={0,0,0});
};

#endif // GAMEOBJECT_H
