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

class MoveProcessor {
public:
    MoveProcessor(Player*, RoomMap*, Point3 dir, DeltaFrame*);
    ~MoveProcessor();
    void try_move();
    void move_bound();
    void move_general();
    void color_change_check();
    void init_movement_components();
    void move_components();
    void fall_step();
    void begin_fall_cycle();
    void check_land_first();
    void make_fall_delta();

    void make_root(Block* obj, std::vector<StrongComponent*>& roots);
    bool try_move_component(StrongComponent*);
    bool try_push(StrongComponent*, Point3);

private:
    Player* player_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    Point3 dir_;
    std::vector<std::unique_ptr<StrongComponent>> move_comps_;
    std::vector<Block*> fall_check_;
    std::vector<std::unique_ptr<WeakComponent>> fall_comps_;
};

#endif // MOVEPROCESSOR_H
