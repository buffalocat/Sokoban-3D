#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"
#include "colorcycle.h"

class Component;
class StrongComponent;
class WeakComponent;

class Block: public GameObject {
public:
    Block(Point3 pos, ColorCycle color, bool car);
    virtual ~Block() = 0;
    virtual void serialize(MapFileO& file);

    unsigned char color();
    void insert_color(unsigned char color);
    bool cycle_color(bool undo);

    virtual std::unique_ptr<StrongComponent> make_strong_component(RoomMap*);
    virtual std::unique_ptr<WeakComponent> make_weak_component(RoomMap*);
    StrongComponent* s_comp();
    WeakComponent* w_comp();
    void reset_comp();

    void set_z(int z);

    virtual bool sticky();
    virtual void get_weak_links(RoomMap*, std::vector<Block*>& links);
    virtual bool has_weak_neighbor(RoomMap* room_map);

    bool car();

    virtual void draw(GraphicsManager*);

protected:
    Component* comp_;
    bool car_;

private:
    ColorCycle color_;
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
    void get_weak_links(RoomMap*, std::vector<Block*>& links);

    void draw(GraphicsManager*);
};

class StickyBlock: public Block {
public:
    StickyBlock(Point3 pos, ColorCycle color, bool car);
    ~StickyBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    bool sticky();
    void get_weak_links(RoomMap*, std::vector<Block*>& links);

    std::unique_ptr<StrongComponent> make_strong_component(RoomMap*);

    void draw(GraphicsManager*);
};

#endif // BLOCK_H
