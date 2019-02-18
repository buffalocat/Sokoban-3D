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
class PushComponent;

// Base class of all objects that occupy a tile in a RoomMap
class GameObject {
public:
    virtual ~GameObject();
    //virtual ObjCode obj_code() const = 0;

    virtual void serialize(MapFileO& file) const;
    virtual bool relation_check() const;
    virtual void relation_serialize(MapFileO& file) const;

    //TODO: decide which of these getters are useful
    Point3 pos() const;
    Point2 posh() const;
    int z() const;
    Point3 shifted_pos(Point3 d) const;

    //NOTE: draw might not be physically const after some optimization!
    virtual void draw(GraphicsManager*, Point3 p);

    virtual void setup_on_put(RoomMap*);
    virtual void cleanup_on_take(RoomMap*);

    virtual void setup_on_undestruction(RoomMap*);
    virtual void cleanup_on_destruction(RoomMap*);

    virtual void collect_sticky_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links) const = 0;
    virtual void collect_special_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links) const;
    virtual bool pretend_push(Point3 d);

    ObjectModifier* modifier();

    //TODO: fix
    void reset_animation();
    void set_linear_animation(Point3);
    void update_animation();
    void shift_pos_from_animation();
    FPoint3 real_pos();

    virtual void collect_strong_component(RoomMap*, PushComponent*, Point3 dir, std::unordered_map<GameObject*, PushComponent*>& push_comps);

protected:
    GameObject(Point3 pos, int color, bool pushable, bool gravitable);

// Data members
protected:
    std::unique_ptr<ObjectModifier> modifier_;
    std::unique_ptr<Animation> animation_;
public:
    Point3 pos_;
    int id_;
    int color_;
    bool pushable_;
    bool gravitable_;
};

#endif // GAMEOBJECT_H
