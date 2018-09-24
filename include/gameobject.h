#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "common.h"

class Shader;
class DeltaFrame;
class RoomMap;

// The GameObject class will really act like the base class
// for Solid objects, until some non-solid objects exist too.
/** Abstract class for all objects that get placed on the map
 */
class GameObject {
public:
    virtual ~GameObject();
    virtual ObjCode obj_code() = 0;
    virtual Layer layer() = 0;
    virtual void serialize(std::ofstream& file);
    virtual bool relation_check();
    virtual void relation_serialize(std::ofstream& file);
    Point pos() const;
    Point shifted_pos(Point) const;
    virtual void draw(Shader*) = 0;

    /// Called when an existing object is "revived" via undo
    // Hence it doesn't need a DeltaFrame
    virtual void reinit();
    /// Called when an object is destroyed
    virtual void cleanup(DeltaFrame*);

    // If layer == Solid && !wall(), then downcast to Block is safe
    virtual bool wall();

protected:
    GameObject(int x, int y);
    Point pos_;
};

/** An immovable, static, Solid layer obstacle
 */
class Wall: public GameObject {
public:
    Wall(int x, int y);
    ~Wall();
    Layer layer();
    ObjCode obj_code();
    bool wall();
    static GameObject* deserialize(unsigned char* buffer);

    void draw(Shader*);
};

#endif // GAMEOBJECT_H
