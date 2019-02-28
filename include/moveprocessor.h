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

struct Component {
    virtual ~Component();

    std::vector<GameObject*> blocks_;
};

struct PushComponent: public Component {
    void add_pushing(Component* comp) {pushing_.push_back(static_cast<PushComponent*>(comp));}

    std::vector<PushComponent*> pushing_;
    bool blocked_;
    bool moving_;
};

struct FallComponent: Component {
    void add_above(Component* comp) {above_.push_back(static_cast<FallComponent*>(comp));}

    void settle_first();

    void take_falling(RoomMap* room_map);

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

    void check_land_first(FallComponent* comp);
    void collect_above(FallComponent* comp, std::vector<GameObject*>& above_list);
    bool drop_check(FallComponent* comp);
    void check_land_sticky(FallComponent* comp);
    void handle_fallen_blocks(FallComponent* comp);
    void settle(FallComponent* comp);

    bool update();
    void abort();

private:
    std::vector<std::unique_ptr<PushComponent>> push_comps_unique_;
    std::vector<std::unique_ptr<FallComponent>> fall_comps_unique_;

    std::vector<GameObject*> moving_blocks_;
    std::vector<GameObject*> fall_check_;
    std::vector<SnakeBlock*> link_break_check_;
    std::vector<SnakeBlock*> moving_snakes_;
    std::vector<SnakeBlock*> snakes_to_reset_;
    std::vector<SnakeBlock*> snakes_to_recheck_;

    Player* player_;
    RoomMap* map_;
    DeltaFrame* delta_frame_;
    Point3 dir_;

    int layers_fallen_;

    unsigned int frames_;
    MoveStepType state_;
};

#endif // MOVEPROCESSOR_H
