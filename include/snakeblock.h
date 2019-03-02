#ifndef SNAKEBLOCK_H
#define SNAKEBLOCK_H

#include <memory>
#include <unordered_set>


#include "common.h"
#include "gameobject.h"

class SnakeBlock: public GameObject {
public:
    SnakeBlock(Point3 pos, int color, bool pushable, bool gravitable, int ends);
    virtual ~SnakeBlock();

    virtual std::string name();
    virtual ObjCode obj_code();
    void serialize(MapFileO& file);
    static std::unique_ptr<GameObject> deserialize(MapFileI& file);
    bool relation_check();
    void relation_serialize(MapFileO& file);

    void collect_sticky_links(RoomMap*, Sticky sticky_level, std::vector<GameObject*>& links);

    bool in_links(SnakeBlock* sb);
    void add_link(SnakeBlock*, DeltaFrame*);
    void add_link_quiet(SnakeBlock*);
    void add_link_one_way(SnakeBlock*);
    void remove_link(SnakeBlock*, DeltaFrame*);
    void remove_link_quiet(SnakeBlock*);
    void remove_link_one_way(SnakeBlock*);

    bool can_link(SnakeBlock*);

    void draw(GraphicsManager*);

    bool available();
    bool confused(RoomMap*);
    void collect_maybe_confused_neighbors(RoomMap*, std::unordered_set<SnakeBlock*>& check);
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

    virtual void cleanup_on_destruction(RoomMap*);
    virtual void setup_on_undestruction(RoomMap*);

    std::unique_ptr<SnakeBlock> make_split_copy();

    Sticky sticky();

    std::vector<SnakeBlock*> links_;
    int ends_;

private:
    // Temporary data members, which should only be nontrivial during move computations
    SnakeBlock* target_;
    unsigned int distance_;

    friend class SnakePuller;
};

class SnakePuller {
public:
    SnakePuller(RoomMap*, DeltaFrame*,
                std::vector<GameObject*>& moving_blocks,
                std::unordered_set<SnakeBlock*>& link_add_check,
                std::vector<GameObject*>& fall_check);
    ~SnakePuller();
    void prepare_pull(SnakeBlock*);
    void perform_pulls();

private:
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    std::vector<SnakeBlock*> snakes_to_pull_;
    std::vector<GameObject*>& moving_blocks_;
    std::unordered_set<SnakeBlock*>& link_add_check_;
    std::vector<GameObject*>& fall_check_;
};

#endif // SNAKEBLOCK_H
