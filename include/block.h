#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"
#include "colorcycle.h"

class Component;

class Block: public GameObject {
public:
    Block(Point3 pos, unsigned char color, bool is_car);
    virtual ~Block() = 0;
    virtual void serialize(MapFileO& file);

    unsigned char color();
    void insert_color(unsigned char color);
    bool cycle_color(bool undo);

    virtual std::unique_ptr<Component> make_strong_component(RoomMap*);
    Component* comp();
    void reset_comp();

    virtual bool sticky();
    virtual void get_weak_links(RoomMap*, std::vector<Block*>& links);

    bool car();

protected:
    Component* comp_;
    bool car_;

private:
    ColorCycle color_;
};

class NonStickBlock: public Block {
public:
    NonStickBlock(Point3 pos, unsigned char color, bool is_car);
    ~NonStickBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    void draw(GraphicsManager*);
};

class WeakBlock: public Block {
public:
    WeakBlock(Point3 pos, unsigned char color, bool is_car);
    ~WeakBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    bool sticky();
    void get_weak_links(RoomMap*, std::vector<Block*>& links);

    //void draw(GraphicsManager*);
};

class StickyBlock: public Block {
public:
    StickyBlock(Point3 pos, unsigned char color, bool is_car);
    ~StickyBlock();
    ObjCode obj_code();
    static GameObject* deserialize(MapFileI& file);

    bool sticky();
    void get_weak_links(RoomMap*, std::vector<Block*>& links);

    //void draw(GraphicsManager*);

    std::unique_ptr<Component> make_strong_component(RoomMap*);
};

#endif // BLOCK_H
