#ifndef BLOCK_H
#define BLOCK_H

#include "common.h"
#include "gameobject.h"

class Component;

class ColorCycle {
public:
    ~ColorCycle();

private:
    ColorCycle(unsigned char color);
    unsigned char color();
    void insert_color(unsigned char color);
    void cycle(bool undo);
    // 5 is the maximum number of colors a block will have (probably)
    // This limit won't be stored in .maps, so it can be changed
    unsigned char color_[5];
    unsigned char size_;
    unsigned char index_;

    friend MapFileO;
    friend Block;
};

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
    bool car_;
    Component* comp_;

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
