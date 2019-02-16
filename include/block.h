#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"
#include "colorcycle.h"

class Component;
class Animation;

enum class Sticky : uint8_t {
    // Stickiness levels of particular objects
    NonStickBlock = 0,
    WeakBlock = 1,
    StrongBlock = 3,
    SnakeBlock = 4,
    // Conditions for sticking
    WEAKSTICK = 1,
    STRONGSTICK = 2,
};

Sticky operator &(Sticky a, Sticky b) {
    return static_cast<Sticky>(static_cast<uint8_t>(a) &
                               static_cast<uint8_t>(b));
}

class Block: public GameObject {
public:
    Block(Point3 pos, ColorCycle color, bool car);
    virtual ~Block() = 0;
    virtual void serialize(MapFileO& file);

    unsigned char color();
    void insert_color(unsigned char color);
    bool cycle_color(bool undo);

    void set_z(int z);

    virtual bool sticky();
    virtual void get_weak_links(RoomMap*, std::vector<Block*>& links);
    virtual bool has_weak_neighbor(RoomMap* room_map);

    bool car();

    void reset_animation();
    void set_linear_animation(Point3);
    void update_animation();
    void shift_pos_from_animation();
    FPoint3 real_pos();
    virtual void draw(GraphicsManager*);

    //NOTE: should these be virtual in Block? maybe!
    bool pushable();
    bool gravitable();

    Sticky sticky_;

protected:
    std::unique_ptr<Animation> animation_;
    ColorCycle color_;
    bool car_;
};

class NonStickBlock: public Block {
public:
    NonStickBlock(Point3 pos, ColorCycle color, bool car);
    ~NonStickBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    void draw(GraphicsManager*);
};

class WeakBlock: public Block {
public:
    WeakBlock(Point3 pos, ColorCycle color, bool car);
    ~WeakBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    bool sticky();

    void draw(GraphicsManager*);
};

class StickyBlock: public Block {
public:
    StickyBlock(Point3 pos, ColorCycle color, bool car);
    ~StickyBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    bool sticky();

    void draw(GraphicsManager*);
};

#endif // BLOCK_H
