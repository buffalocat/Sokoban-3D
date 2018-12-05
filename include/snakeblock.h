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
    void root_init(Point3);
    void get_weak_links(RoomMap*, std::vector<Block*>&);

    bool in_links(SnakeBlock* sb);
    void add_link(SnakeBlock*, DeltaFrame*);
    void remove_link(SnakeBlock*, DeltaFrame*);

    void draw(GraphicsManager*);

    bool available();
    bool confused(RoomMap*);
    void check_add_local_links(RoomMap*, DeltaFrame*);
    void check_remove_local_links(DeltaFrame*);
    //void collect_unlinked_neighbors(RoomMap*, std::)
    void reset_target();

    void cleanup();
    void reinit();

private:
    std::vector<SnakeBlock*> links_;
    SnakeBlock* target_;
    unsigned int distance_;
    unsigned char ends_;

    friend class SnakeComponent;
    friend class SnakePuller;
};

class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*, Point3,
                std::vector<SnakeBlock*>& check_snakes,
                std::vector<GameObject*>& below_release);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void pull(SnakeBlock*);

private:
    RoomMap* room_map_;
    DeltaFrame* delta_frame_;
    std::vector<SnakeBlock*>& check_snakes_;
    std::vector<GameObject*>& below_release_;
    Point3 dir_;
};

#endif // SNAKEBLOCK_H
