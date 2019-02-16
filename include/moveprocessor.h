#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "common.h"

class Player;
class Block;
class RoomMap;
class DeltaFrame;
class SnakeBlock;

enum class MoveStepType {
    Horizontal = 1,
    Fall = 2,
};

struct PushComponent {
    std::vector<Block*> blocks_;
    std::unordered_set<PushComponent*> pushing_;
    bool blocked_;
    bool pushed_;
    PushComponent(bool pushed): blocks_ {}, pushing_ {}, blocked_ {false}, pushed_ {pushed} {}
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

    bool try_move_component(StrongComponent*);
    bool try_push(StrongComponent*, Point3);

private:
    Player* player_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    Point3 dir_;
    std::vector<std::unique_ptr<PushComponent>> push_comps_unique_;
    std::unordered_map<Block*, PushComponent*> push_comps_;
    std::vector<PushComponent*> moving_comps_;

    std::vector<std::unique_ptr<StrongComponent>> move_comps_;
    std::vector<Block*> moving_blocks_;
    std::vector<Block*> fall_check_;
    std::vector<SnakeBlock*> link_add_check_;
    std::vector<SnakeBlock*> link_break_check_;
    std::vector<std::unique_ptr<WeakComponent>> fall_comps_;

    unsigned int frames_;
    MoveStepType state_;
};

#endif // MOVEPROCESSOR_H
