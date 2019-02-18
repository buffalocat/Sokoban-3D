#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "common.h"

class Player;
class GameObject;
class RoomMap;
class DeltaFrame;
class SnakeBlock;

enum class MoveStepType {
    Horizontal = 1,
    Fall = 2,
};

struct PushComponent {
    PushComponent(): blocks_ {}, pushing_ {}, blocked_ {false}, moving_ {false} {}
    std::vector<GameObject*> blocks_;
    std::vector<PushComponent*> pushing_;
    bool blocked_;
    bool moving_;
};

struct FallComponent {
    void check_land_first(RoomMap*);
    void settle_first();

    void collect_above(std::vector<GameObject*>& above_list, RoomMap* room_map);
    void collect_falling_unique(RoomMap* room_map);

    // Make these methods of MoveProcessor!
    bool drop_check(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);
    void check_land_sticky(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);
    void handle_unique_blocks(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);
    void settle(int layers_fallen, RoomMap* room_map, DeltaFrame* delta_frame);

    std::vector<GameObject*> blocks_;
    std::vector<FallComponent*> above_;
    bool settled_;
};

class MoveProcessor {
public:
    MoveProcessor(Player*, RoomMap*, Point3 dir, DeltaFrame*);
    ~MoveProcessor();

    bool try_move();
    void move_bound();
    void move_general();
    void color_change_check();

    void prepare_horizontal_move();
    void perform_horizontal_step();

    bool compute_push_component_tree(GameObject* block);
    bool compute_push_component(GameObject* block);

    void collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links);

    void fall_step();
    void perform_switch_checks();
    void begin_fall_cycle();
    /*
    void check_land_first();
    void make_fall_delta();
    */

    bool update();
    void abort();

private:
    std::unordered_map<GameObject*, PushComponent*> push_comps_;
    std::vector<std::unique_ptr<PushComponent>> push_comps_unique_;

    std::vector<GameObject*> moving_blocks_;
    std::vector<GameObject*> fall_check_;
    std::vector<SnakeBlock*> link_break_check_;

    std::unordered_map<GameObject*, FallComponent*> fall_comps_;
    std::vector<std::unique_ptr<FallComponent>> fall_comps_unique_;

    std::vector<SnakeBlock*> moving_snakes_;
    std::vector<SnakeBlock*> snakes_to_reset_;
    std::vector<SnakeBlock*> snakes_to_recheck_;

    Player* player_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    Point3 dir_;

    unsigned int frames_;
    MoveStepType state_;
};

#endif // MOVEPROCESSOR_H
