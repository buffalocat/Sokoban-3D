#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <vector>
#include <memory>

#include "common.h"

class ObjectModifier;
class Animation;
class DeltaFrame;
class RoomMap;
class GraphicsManager;
class MapFileI;
class MapFileO;

struct Component;
struct PushComponent;
struct FallComponent;

// Base class of all objects that occupy a tile in a RoomMap
class GameObject {
public:
    virtual ~GameObject();

    virtual ObjCode obj_code() = 0;
    virtual void serialize(MapFileO& file) = 0;
    virtual bool relation_check();
    virtual void relation_serialize(MapFileO& file);

    Point3 shifted_pos(Point3 d);

    // TODO: make this pure virtual, with no Point arg
    virtual void draw(GraphicsManager*, Point3) {}

    virtual void setup_on_put(RoomMap*);
    virtual void cleanup_on_take(RoomMap*);

    virtual void setup_on_undestruction(RoomMap*);
    virtual void cleanup_on_destruction(RoomMap*);

    PushComponent* push_comp();
    FallComponent* fall_comp();

    void collect_sticky_component(RoomMap*, Sticky, Component*);
    virtual Sticky sticky() = 0;
    bool has_sticky_neighbor(RoomMap*);
    virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>& links) = 0;
    virtual void collect_special_links(RoomMap*, Sticky, std::vector<GameObject*>& links);

    ObjectModifier* modifier();

    //TODO: fix
    void reset_animation();
    void set_linear_animation(Point3);
    void update_animation();
    void shift_pos_from_animation();
    FPoint3 real_pos();

protected:
    GameObject(Point3 pos, unsigned char color, bool pushable, bool gravitable);

// Data members
protected:
    std::unique_ptr<ObjectModifier> modifier_;
    std::unique_ptr<Animation> animation_;
public:
    Component* comp_;
    Point3 pos_;
    int id_;
    unsigned char color_;
    // TODO: remove this later
    unsigned char color() {return color_;}
    bool pushable_;
    bool gravitable_;
};

#endif // GAMEOBJECT_H
