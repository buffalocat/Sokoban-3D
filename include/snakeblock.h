#ifndef SNAKEBLOCK_H
#define SNAKEBLOCK_H

#include "common.h"
#include "block.h"

class StrongComponent;
class WeakComponent;

/** A different type of block which forms "chains", represented by a diamond
 */
class SnakeBlock: public Block {
public:
    SnakeBlock(Point3 pos, ColorCycle color, bool is_car, unsigned char ends);
    virtual ~SnakeBlock();
    virtual ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    bool relation_check();
    void relation_serialize(MapFileO& file);

    std::unique_ptr<StrongComponent> make_strong_component(RoomMap*);
    std::unique_ptr<WeakComponent> make_weak_component(RoomMap*);

    bool in_links(SnakeBlock* sb)
    void add_link(SnakeBlock*, DeltaFrame*);
    void remove_link(SnakeBlock*, DeltaFrame*);

    void draw(GraphicsManager*);

    bool available();
    bool confused(RoomMap*);
    void check_local_links(RoomMap*, DeltaFrame*);

private:
    std::vector<SnakeBlock*> links_;
    unsigned char ends_;
    unsigned int dist_;
    SnakeComponent* target

    friend class SnakeComponent;
};

class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*, std::vector<std::pair<SnakeBlock*, Point3>>&, std::vector<SnakeBlock*>&);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void pull(SnakeBlock*);

private:
    RoomMap* room_map_;
    DeltaFrame* delta_frame_;
    std::vector<std::pair<SnakeBlock*, Point3>>& pull_snakes_;
    std::vector<SnakeBlock*>& check_snakes_;
};

#endif // SNAKEBLOCK_H
