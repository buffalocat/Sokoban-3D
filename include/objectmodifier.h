#ifndef OBJECTMODIFIER_H
#define OBJECTMODIFIER_H

#include <vector>

#include "common.h"
#include "point.h"

class GameObject;
class RoomMap;

// Base class of object modifiers such as Car, Door, Switch, and Gate
class ObjectModifier {
public:
    ObjectModifier(GameObject* parent);
    virtual ~ObjectModifier();

    GameObject* parent_;

    Point3 pos() const;
    Point3 shifted_pos(Point3 d) const;
    int color() const;
    bool pushable() const;
    bool gravitable() const;
    //TODO: put in generic functions that all modifiers need to have!

    virtual void collect_sticky_links(RoomMap*, Sticky, std::vector<GameObject*>&) const;

};

#endif // OBJECTMODIFIER_H
