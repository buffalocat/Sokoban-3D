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
    Point shifted_pos(Point d) const;
    void set_pos_auto(Point p, RoomMap* room_map, DeltaFrame* delta_frame);
    void set_pos(Point p);
    void shift_pos_auto(Point d, RoomMap* room_map, DeltaFrame* delta_frame);

    virtual void draw(Shader*) = 0;

    /// Called when an existing object is "revived" via undo
    // Hence it doesn't need a DeltaFrame
    virtual void reinit();
    /// Called when an object is destroyed
    virtual void cleanup(DeltaFrame*);

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
    static GameObject* deserialize(unsigned char* buffer);

    void draw(Shader*);
};

// The Player object behaves differently from all other objects with respect to save/load.
// Every map should have one Player position designated, but this is overridden if
// the map is entered via a door, in which case that door's starting position is used.
// Thus, we don't actually serialize the player to the map as an object - just a position.
class Player: public GameObject {
public:
    Player(int x, int y, RidingState state);
    ~Player();
    ObjCode obj_code();
    Layer layer();
    void serialize(std::ofstream& file);
    static GameObject* deserialize(unsigned char* buffer);
    RidingState state();
    Block* get_car(RoomMap* room_map);

    void draw(Shader*);

private:
    RidingState state_;
};

class PlayerWall: public GameObject {
public:
    PlayerWall(int x, int y);
    ~PlayerWall();
    Layer layer();
    ObjCode obj_code();
    static GameObject* deserialize(unsigned char* buffer);

    void draw(Shader*);
};

#endif // GAMEOBJECT_H
