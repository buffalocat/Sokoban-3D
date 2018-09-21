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
    static std::unordered_map<size_t, ObjCode> code_map;
    static std::unordered_map<int, GameObject* (*)(unsigned char*)> deser_map;

    virtual ~GameObject() = 0;
    virtual ObjCode obj_code();
    virtual void serialize(std::ofstream& file) = 0;
    Layer layer() const;
    Point pos() const;
    Point shifted_pos(Point) const;
    virtual void draw(Shader*) = 0;
    virtual void cleanup(DeltaFrame*) = 0;
    virtual void reinit() = 0;
    // If not wall(), then downcast to Block is safe
    bool wall() const;

protected:
    GameObject(int x, int y);
    Point pos_;
    bool wall_;
};

/** An immovable, static, Solid layer obstacle
 */
class Wall: public GameObject {
public:
    Wall(int x, int y);
    ~Wall();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    void draw(Shader*);
    void cleanup(DeltaFrame*);
    void reinit();
};

#endif // GAMEOBJECT_H
