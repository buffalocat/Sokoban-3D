#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "common.h"

class Player;
class Block;
class RoomMap;
class DeltaFrame;
class SnakeBlock;
class StrongComponent;
class WeakComponent;

enum class MoveState {
    Horizontal = 1,
    Fall = 2,
};

class MoveProcessor {
public:
    MoveProcessor(Player*, RoomMap*, Point3 dir, DeltaFrame*);
    ~MoveProcessor();
    bool try_move();
    void move_bound();
    void move_general();
    void color_change_check();
    void init_movement_components();
    void move_components();
    void fall_step();
    void perform_switch_checks();
    void begin_fall_cycle();
    void check_land_first();
    void make_fall_delta();

    bool update();
    void abort();

    void make_root(Block* obj, std::vector<StrongComponent*>& roots);
    bool try_move_component(StrongComponent*);
    bool try_push(StrongComponent*, Point3);

private:
    Player* player_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    Point3 dir_;
    std::vector<std::unique_ptr<StrongComponent>> move_comps_;
    std::vector<GameObject*> below_release_;
    std::vector<GameObject*> below_press_;
    std::vector<Block*> moving_blocks_;
    std::vector<Block*> fall_check_;
    std::vector<SnakeBlock*> link_add_check_;
    std::vector<SnakeBlock*> link_break_check_;
    std::vector<std::unique_ptr<WeakComponent>> fall_comps_;

    unsigned int frames_;
    MoveState state_;
};

#endif // MOVEPROCESSOR_H
