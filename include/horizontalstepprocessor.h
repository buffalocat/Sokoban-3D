#ifndef HORIZONTALSTEPPROCESSOR_H
#define HORIZONTALSTEPPROCESSOR_H

#include <vector>
#include <memory>

#include "point.h"

#include "component.h"

class GameObject;
class SnakeBlock;
class Player;

class RoomMap;
class DeltaFrame;

class HorizontalStepProcessor {
public:
    HorizontalStepProcessor(RoomMap*, DeltaFrame*, Point3, std::vector<GameObject*>&, std::vector<GameObject*>&);
    ~HorizontalStepProcessor();

    // NOTE: We can probably eliminate player from here eventually
    // When the map can provide a list of all moving agents
    void run(Player*);

private:
    void prepare_horizontal_move();
    void perform_horizontal_step();

    bool compute_push_component_tree(GameObject* block);
    bool compute_push_component(GameObject* block);

    void collect_moving_and_weak_links(PushComponent* comp, std::vector<GameObject*>& weak_links);

    std::vector<std::unique_ptr<PushComponent>> push_comps_unique_;

    std::vector<SnakeBlock*> link_break_check_;
    std::vector<SnakeBlock*> moving_snakes_;
    std::vector<SnakeBlock*> snakes_to_reset_;
    std::vector<SnakeBlock*> snakes_to_recheck_;
    std::vector<GameObject*>& moving_blocks_;
    std::vector<GameObject*>& fall_check_;

    RoomMap* map_;
    DeltaFrame* delta_frame_;
    Point3 dir_;
};

#endif // HORIZONTALSTEPPROCESSOR_H
