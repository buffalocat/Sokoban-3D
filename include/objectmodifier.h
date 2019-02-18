#ifndef OBJECTMODIFIER_H
#define OBJECTMODIFIER_H

#include <vector>

#include "common.h"
#include "point.h"

class GameObject;
class RoomMap;
class DeltaFrame;
class MoveProcessor;

// Base class of object modifiers such as Car, Door, Switch, and Gate
class ObjectModifier {
public:
    ObjectModifier(GameObject* parent);
    virtual ~ObjectModifier();

    GameObject* parent_;

    Point3 pos() const;
    Point3 shifted_pos(Point3 d) const;
    Point3 pos_above() const; // A convenience function often needed by Modifiers
    int color() const;
    bool pushable() const;
    bool gravitable() const;

    // Every type of Modifier can have at most one callback function for map listeners
    virtual void map_callback(RoomMap*, DeltaFrame*, MoveProcessor*) = 0;
    virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&) const;

};

#endif // OBJECTMODIFIER_H
