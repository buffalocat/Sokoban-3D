#ifndef SNAKEBLOCK_H
#define SNAKEBLOCK_H

#include "common.h"
#include "block.h"

/** A different type of block which forms "chains", represented by a diamond
 */
class SnakeBlock: public Block {
public:
    SnakeBlock(Point3 pos, unsigned char color, bool is_car, unsigned char ends);
    virtual ~SnakeBlock();
    virtual ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    bool relation_check();
    void relation_serialize(MapFileO& file);

    void add_link(SnakeBlock*, DeltaFrame*);
    void remove_link(SnakeBlock*, DeltaFrame*);

    void draw(GraphicsManager*);
/*
    bool available();
    bool confused(RoomMap*);
    void check_add_local_links(RoomMap*, DeltaFrame*);
    void post_move_reset();
    */

private:
    unsigned char ends_;
    std::vector<SnakeBlock*> links_;

    friend class SnakePuller;
};

class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*, Point);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void pull(SnakeBlock*);

private:
    RoomMap* room_map_;
    DeltaFrame* delta_frame_;
    Point dir_;
};

#endif // SNAKEBLOCK_H
