#ifndef SNAKEBLOCK_H
#define SNAKEBLOCK_H

#include <unordered_set>

#include "common.h"
#include "gameobject.h"

class SnakeBlock: public GameObject {
public:
    SnakeBlock(Point3 pos, int color, bool pushable, bool gravitable, unsigned char ends);
    virtual ~SnakeBlock();
    virtual ObjCode obj_code();
    void serialize(MapFileO& file);
    static GameObject* deserialize(MapFileI& file);
    bool relation_check();
    void relation_serialize(MapFileO& file);

    void collect_sticky_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    bool in_links(SnakeBlock* sb);
    void add_link(SnakeBlock*, DeltaFrame*);
    void remove_link(SnakeBlock*, DeltaFrame*);

    void draw(GraphicsManager*);

    bool available();
    bool confused(RoomMap*);
    void collect_maybe_confused_links(RoomMap*, std::unordered_set<SnakeBlock*>& check);
    void update_links_color(RoomMap*, DeltaFrame*);
    void check_add_local_links(RoomMap*, DeltaFrame*);
    void remove_moving_links(DeltaFrame*);

    // distance_ encodes information about the state of the block during move processing
    // A snake which is moving has positive distance_
    // A snake which was not pushed has disitance_ at least 2
    // target_ is only changed during snake pulling, and is reset afterwards
    void toggle_push();
    void record_move();
    void reset_distance_and_target();
    bool pushed_and_moving();

    void cleanup();
    void reinit();

    Sticky sticky();

private:
    std::vector<SnakeBlock*> links_;
    // Temporary data members, which should only be nontrivial during move computations
    SnakeBlock* target_;
    unsigned int distance_;

    unsigned char ends_;

    friend class SnakePuller;
};

class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*,
                std::vector<SnakeBlock*>& moving_snakes,
                std::unordered_set<SnakeBlock*>& add_link_check,
                std::vector<GameObject*>& fall_check);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void perform_pulls();

private:
    RoomMap* room_map_;
    DeltaFrame* delta_frame_;
    std::vector<SnakeBlock*>& moving_snakes_;
    std::unordered_set<SnakeBlock*>& add_link_check_;
    std::vector<GameObject*>& fall_check_;
    Point3 dir_;
};

#endif // SNAKEBLOCK_H
